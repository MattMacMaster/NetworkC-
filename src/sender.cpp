#include "../include/PacketHeader.h"
#include <arpa/inet.h>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <zlib.h>

/*
 * Using aliases to make code more readable - bytes
 */
using BYTE = unsigned char;
using BYTES = unsigned char *;
using CONST_BYTES = const unsigned char *;
/*
 * Using aliases to make code more readable - strs
 */
using CONST_CHAR_STR = const char *;
const uint32_t MAXLINE = 4096;
const uint32_t BUFFER_SIZE = 16;
const uint32_t ZERO = 0;
const uint32_t HEADER_OFFSET = 12;

/*
 * Using an enum to manage segment type
 */
enum SegmentType {
  PTYPE_DATA = 1,
  PTYPE_ACK = 2,
  PTYPE_NACK = 3,
};

auto empty_buffer_factory() -> BYTES {
  /* Function to initialize basic buffer
   *
   * Description:
   * - Initializes and sets the buffer values to zero
   * - Initializes packet header to build the buffer
   * - Sets the type of the packet header to PTYPE_DATA i.e. the indication that
   * the packet contains data
   * - TR was initailly set to false, but since zero is functionally the same
   * thing in c/cpp, removed that part of the code
   *
   * - Initializes a temporary value to crc1 and adds it to the buffer
   *
   * Args:
   * - None
   *
   * Returns:
   *  buffer (unsigned char *) the packet header
   *
   * Throws:
   *  Nothing
   *
   */
  BYTES buffer;
  PacketHeader header;
  memset(buffer, 0, BUFFER_SIZE); // rm-rf where buffer's set
  header.setType(buffer, PTYPE_DATA);
  uint32_t crc1;

  crc1 = crc32(crc1, buffer, HEADER_OFFSET); // initializing crc1
  header.setCRC(buffer, crc1);               // setting buffer crc1 area to crc1

  return buffer;
}

auto payload_factory(CONST_CHAR_STR payload, size_t payload_len) -> BYTES {
  /* Function to add the payload to the buffer
   *
   * Description:
   * - Initializes:
   *    - payload_length
   *    - header
   *    - buffer
   * - Clears the memory that buffer has been set to.
   * - Sets type of header as PTYPE_DATA, since the packet will contain
   *   the data from the payload.
   * - Sets the len of the header payload to the payload length, payload needs
   *   to be casted to a uint, though
   * - Computes the CRC1 value based on the buffer as it is
   *
   * - Copies the payload and payload length to the buffer
   * - Computes crc2 and adds it to the buffer
   *
   * Args:
   *
   *
   * Returns:
   * - buffer (unsigned char *)
   *
   *
   * Throws:
   *  - Nothing
   *
   */

  // declaring variables
  uint payload_length = static_cast<uint>(payload_len);
  CONST_BYTES byte_string_payload = reinterpret_cast<CONST_BYTES>(payload);
  PacketHeader header;
  BYTES buffer;
  memset(buffer, ZERO, BUFFER_SIZE + payload_len);
  uint32_t crc1;
  crc1 = crc32(crc1, buffer, HEADER_OFFSET);
  size_t crc2_offset = HEADER_OFFSET + payload_len;

  // Using packetheader to build the buffer
  header.setType(buffer, PTYPE_DATA);
  header.setLen(buffer, payload_length);
  header.setCRC(buffer, crc1);

  // copying data from payload into buffer
  if (payload_len > ZERO)
    memcpy(buffer + HEADER_OFFSET, payload, payload_len);

  uint32_t crc2 = crc32(0L, Z_NULL, ZERO);
  if (payload_len > ZERO)
    crc2 = crc32(crc2, byte_string_payload, payload_len);

  // going to create a new func in packet header and extract this into it
  buffer[crc2_offset] = crc2 & 0xFF;
  buffer[crc2_offset + 1] = (crc2 >> 8) & 0xFF;
  buffer[crc2_offset + 2] = (crc2 >> 16) & 0xFF;
  buffer[crc2_offset + 3] = (crc2 >> 24) & 0xFF;

  return buffer;
}

auto read_file(CONST_CHAR_STR file_name) -> std::vector<BYTE> {
  /* Function to read the file
   *
   * Description:
   *  This function reads the data from a provided file
   *
   * Args:
   *  file_name (const char *): The name of the file to be sent
   *
   * Returns:
   *  data within file (std::vector<unsigned char>)
   *
   * Throws:
   *  Nothing
   *
   */
  std::ifstream file(file_name, std::ios::binary);

  file.seekg(0, std::ios::end);
  std::streampos file_size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<BYTE> fileData(file_size);
  file.read((char *)&fileData[0], file_size);
  return fileData;
}

// Driver code
void ipv4UDP(CONST_CHAR_STR hostname, int port, CONST_CHAR_STR file) {
  // Initializing socket
  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Initializing server address & information
  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  servaddr.sin_addr.s_addr = inet_addr(hostname);

  std::vector<BYTE> payload = read_file(file);
  CONST_CHAR_STR payload_data = reinterpret_cast<const char *>(payload.data());
  size_t buffer_size = HEADER_OFFSET + 4 + payload.size();
  BYTE *buffer = new BYTE[buffer_size];
  buffer = payload_factory(payload_data, payload.size());

  sendto(sockfd, (const char *)buffer, payload.size(), 0,
         (const struct sockaddr *)&servaddr, sizeof(servaddr));

  close(sockfd);
}

void ipv6UDP(const char *hostname, int port, const char *file) {
  // Initializing socket & socket file descriptor
  int sockfd;

  if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  // Initializing server & server address
  struct sockaddr_in6 servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin6_family = AF_INET6; // IPv6
  servaddr.sin6_port = htons(port);

  // Use the loopback address for IPv6 (::1)
  if (inet_pton(AF_INET6, hostname, &servaddr.sin6_addr) <= 0) {
    perror("Invalid IPv6 address");
    exit(EXIT_FAILURE);
  }

  // Initializing Payload from file data
  char buffer[MAXLINE];
  std::string temp;

  std::vector<BYTE> fileData = read_file(file);
  for (int i = 0; i < fileData.size(); ++i)
    temp.push_back(fileData[i]);

  CONST_CHAR_STR payload = temp.c_str();

  // Send a Packet
  // TODO WE NEED TO USE PA1 here, append it all
  // Then send it
  int bytes_sent = sendto(sockfd, (const char *)payload, fileData.size(), 0,
                          (const struct sockaddr *)&servaddr, sizeof(servaddr));

  if (bytes_sent < 0) {
    perror("Send failed");
  } else {
    std::cout << "Msg sent:" << payload << std::endl;
  }

  close(sockfd);
}

int main(int arg_count, char *args[]) {
  if (arg_count < 3) {
    std::cerr << "Usage: ./sender [hostname] [port] [-f filename]" << std::endl;
    return 1;
  }

  const char *hostname = args[1];
  int port = std::stoi(args[2]);
  const char *filename = "";

  // Parse optional -f flag and filename
  for (int i = 3; i < arg_count; i++) {
    if (std::string(args[i]) == "-f") {
      if (i + 1 < arg_count) {
        filename = args[i + 1]; // Set filename to the next argument
        i++; // Skip next argument since it's already used as filename
      } else {
        std::cerr << "Error: Missing filename after -f flag." << std::endl;
        return 1;
      }
    }
  }
  std::cout << "hostname: " << hostname << std::endl;
  std::cout << "PORT: " << port << std::endl;
  std::cout << "filename: " << filename << std::endl;

  // TODO - detect which one to run
  // And switch to that option
  ipv4UDP(hostname, port, filename);
  // ipv6UDP(hostname,port,filename);

  return 0;
}
