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

using CSTR = const char *;
using BYTE = unsigned char;
using BYTES = unsigned char *;
using CBYTES = const unsigned char *; // just tryna shorten some code and make
                                      // our program slightly more readable

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
};

enum SegmentType {
  PTYPE_DATA = 1,
  PTYPE_ACK = 2,
  PTYPE_NACK = 3,
};

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

void payload_factory(BYTES buffer, CSTR payload, size_t payload_len) {
  memset(buffer, 0, BUFFER_META_DATA_SIZE + payload_len);
  uint32_t payload_length = static_cast<uint32_t>(payload_len);
  uint32_t crc1 = crc32(0L, Z_NULL, 0);

  PacketHeader header;

  header.setType(buffer, SegmentType::PTYPE_DATA);
  header.setTR(buffer, false);
  header.setLen(buffer, payload_length);

  // Compute CRC1 over first 8 bytes (start of type ---> end of Timestamp)
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
  }

  size_t crc2_offset = 12 + payload_len;
  buffer[crc2_offset] = crc2 & 0xFF;
  buffer[crc2_offset + 1] = (crc2 >> 8) & 0xFF;
  buffer[crc2_offset + 2] = (crc2 >> 16) & 0xFF;
  buffer[crc2_offset + 3] = (crc2 >> 24) & 0xFF;
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

  sendto(sockfd, (const char *)buffer, payload_size, 0,
         (const struct sockaddr *)&servaddr, sizeof(servaddr));

  close(sockfd);
}

int main(int argc, char *argv[]) {
  // Parsing command line args:
  SenderArgs sender_args = SenderArgs(argc, argv);
  std::cout << "HOSTNAME: " << sender_args.hostname << std::endl;
  std::cout << "PORT: " << sender_args.port << std::endl;
  std::cout << "FILEPATH: " << sender_args.file_path << std::endl;

  // Passing commandline args to ipv4UDP sender
  ipv4UDP(sender_args.hostname, sender_args.port, sender_args.file_path);
  return 0;
}
