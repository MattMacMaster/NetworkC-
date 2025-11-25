#include "../include/PacketHeader.h"
#include <arpa/inet.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <zlib.h>

namespace fs = std::filesystem;

using BYTES = unsigned char *; // our program slightly more readable

// Using static to make the constants public only to this file
static const uint32_t MAX_PAYLOAD_SIZE = 4096;
static const uint32_t CRC_1_BUFFER_INDEX = 8; // = 12 leads to crc error
static const uint32_t PAYLOAD_START_INDEX = 12;
static const uint32_t BUFFER_META_DATA_SIZE = 16; // size of all but payload

// command line flags
struct SenderArgs {
  const char *hostname;
  uint32_t port;
  fs::path file_path;

  SenderArgs(int arg_count, char *args[]) {
    if (arg_count < 3) {
      std::cerr << "Usage: ./sender [-f filename] [hostname] [port] "
                << std::endl;
      std::exit(1);
    }
    if (!strcmp(args[1], "-f")) {
      file_path = args[2];
      if (!fs::exists(file_path)) {
        throw std::runtime_error("Error, file (path) not found");
      }
      hostname = args[3];
      port = atoi(args[4]);

    } else {
      hostname = args[1];
      port = atoi(args[2]);
    }
  }

  void display() const {
    std::cout << "HOSTNAME: " << hostname << '\n'
              << "PORT: " << port << '\n'
              << "FILEPATH: " << file_path << std::endl;
  }
};

enum IPVersion {
  // Enum for IP Type
  IPv4,
  IPv6,
  UNKNOWN
};

enum SegmentType {
  // Enum for type of data being sent
  PTYPE_DATA = 1,
  PTYPE_ACK = 2,
  PTYPE_NACK = 3,
};

IPVersion detect_ip_version(const std::string &host) {
  struct in_addr addr4;
  struct in6_addr addr6;

  if (inet_pton(AF_INET, host.c_str(), &addr4) == 1) {
    return IPVersion::IPv4;
  } else if (inet_pton(AF_INET6, host.c_str(), &addr6) == 1) {
    return IPVersion::IPv6;
  }
  return IPVersion::UNKNOWN;
}

std::vector<std::byte> read_payload_to_bytes(fs::path file_name) {
  auto length = fs::file_size(file_name);
  if (length == 0) {
    return {};
  }

  std::vector<std::byte> buffer(length);
  std::ifstream file(file_name, std::ios_base::binary);
  file.read(reinterpret_cast<char *>(buffer.data()), length);
  file.close();
  return buffer;
}

void payload_factory(BYTES buffer, const char *payload, size_t payload_len) {
  memset(buffer, 0, BUFFER_META_DATA_SIZE + payload_len);
  uint32_t payload_length = static_cast<uint32_t>(payload_len);

  PacketHeader header;
  header.setType(buffer, SegmentType::PTYPE_DATA);
  header.setTR(buffer, false);
  header.setLen(buffer, payload_length);
  header.setSeq(buffer, 0);
  header.setWin(buffer, 31);

  // Compute CRC1 over first 8 bytes (start of type ---> end of Timestamp)
  uint32_t crc1 = crc32(0L, Z_NULL, 0);
  crc1 = crc32(crc1, buffer, CRC_1_BUFFER_INDEX);
  header.setCRC(buffer, crc1); // Store in little-endian

  // Copy payload immediately after header + CRC1
  if (payload_len > 0) {
    memcpy(buffer + 12, payload, payload_len);
  }
  // Compute CRC2 over the payload
  uint32_t crc2 = crc32(0L, Z_NULL, 0);
  if (payload_len > 0) {
    crc2 = crc32(crc2, reinterpret_cast<const unsigned char *>(payload),
                 payload_len);
    size_t crc2_offset = 12 + payload_len;
    buffer[crc2_offset] = (crc2 >> 24) & 0xFF;
    buffer[crc2_offset + 1] = (crc2 >> 16) & 0xFF;
    buffer[crc2_offset + 2] = (crc2 >> 8) & 0xFF;
    buffer[crc2_offset + 3] = crc2 & 0xFF;
  }
}

void dead_factory(BYTES buffer) {
  memset(buffer, 0, BUFFER_META_DATA_SIZE);

  PacketHeader header;

  header.setType(buffer, SegmentType::PTYPE_DATA);
  header.setTR(buffer, false);
  header.setLen(buffer, 0);
  header.setSeq(buffer, 1);
  header.setWin(buffer, 31);

  // Compute CRC1 over first 8 bytes (start of type ---> end of Timestamp)
  uint32_t crc1 = crc32(0L, Z_NULL, 0);
  crc1 = crc32(crc1, buffer, CRC_1_BUFFER_INDEX);
  header.setCRC(buffer, crc1); // Store in little-endian
}

void ipv4UDP(const char *hostname, int port, fs::path file) {
  std::vector<std::byte> payload = read_payload_to_bytes(file);
  uint32_t payload_size = fs::file_size(file);
  uint32_t buffer_size = BUFFER_META_DATA_SIZE + payload_size;
  unsigned char *buffer = new unsigned char[buffer_size];

  int sockfd;
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  payload_factory(buffer, reinterpret_cast<const char *>(payload.data()),
                  payload.size());

  struct sockaddr_in servaddr;
  memset(&servaddr, 0, sizeof(servaddr));
  // Filling server information
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(port);
  servaddr.sin_addr.s_addr = inet_addr(hostname);

  // THE 1 MATTERS ALOT, its the ammount of expecting packets?
  sendto(sockfd, (const char *)buffer, buffer_size, 0,
         (const struct sockaddr *)&servaddr, sizeof(servaddr));

  sleep(3);
  // Send Dead packet
  uint32_t dead_buffer_size = BUFFER_META_DATA_SIZE;
  unsigned char *deadbuffer = new unsigned char[dead_buffer_size];
  dead_factory(deadbuffer);

  sendto(sockfd, (const char *)deadbuffer, dead_buffer_size, 0,
         (const struct sockaddr *)&servaddr, sizeof(servaddr));

  sleep(3);
  close(sockfd);
}

void ipv6UDP(const char *hostname, int port, fs::path file) {
  std::vector<std::byte> payload = read_payload_to_bytes(file);
  uint32_t payload_size = fs::file_size(file);

  uint32_t buffer_size = BUFFER_META_DATA_SIZE + payload_size;
  unsigned char *buffer = new unsigned char[buffer_size];

  int sockfd;
  if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }

  payload_factory(buffer, reinterpret_cast<const char *>(payload.data()),
                  payload.size());

  struct sockaddr_in6 servaddr; // IPv6 structure
  memset(&servaddr, 0, sizeof(servaddr));
  servaddr.sin6_family = AF_INET6;  // IPv6
  servaddr.sin6_port = htons(port); // Port

  if (inet_pton(AF_INET6, hostname, &servaddr.sin6_addr) <= 0) {
    perror("Invalid IPv6 address");
    exit(EXIT_FAILURE);
  }

  if (sendto(sockfd, buffer, buffer_size, 0, (struct sockaddr *)&servaddr,
             sizeof(servaddr)) < 0) {
    perror("sendto failed");
  }

  sleep(3);
  // Send Dead packet
  uint32_t dead_buffer_size = BUFFER_META_DATA_SIZE;
  unsigned char *deadbuffer = new unsigned char[dead_buffer_size];
  dead_factory(deadbuffer);

  sendto(sockfd, (const char *)deadbuffer, dead_buffer_size, 0,
         (const struct sockaddr *)&servaddr, sizeof(servaddr));

  sleep(3);
  close(sockfd);
  delete[] buffer;
}

int main(int argc, char *argv[]) {
  SenderArgs sender_args = SenderArgs(argc, argv);
  sender_args.display();
  IPVersion version = detect_ip_version(sender_args.hostname);

  if (version == IPVersion::IPv4) {
    std::cout << sender_args.hostname << " is IPv4\n";
    ipv4UDP(sender_args.hostname, sender_args.port, sender_args.file_path);
  } else if (version == IPVersion::IPv6) {
    std::cout << sender_args.hostname << " is IPv6\n";
    ipv6UDP(sender_args.hostname, sender_args.port, sender_args.file_path);
  } else
    std::cout << sender_args.hostname << " is unknown or a hostname\n";
}
