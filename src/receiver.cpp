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

// Using static to make the constants public only to this file
static const uint32_t MAX_PAYLOAD_SIZE = 4096;
static const uint32_t CRC_1_BUFFER_INDEX = 8; // = 12 leads to crc error
static const uint32_t PAYLOAD_START_INDEX = 12;
static const uint32_t BUFFER_META_DATA_SIZE = 16; // size of all but payload

// command line flags
// receiver [-f data_file] [-6] port

enum IPVersion { IPv4, IPv6 };

struct ReceiverArgs {
  IPVersion ip_version;
  fs::path file_path = "";
  uint32_t port;

  ReceiverArgs(int arg_count, char *args[]) {
    std::string file_flag = "-f";
    std::string ipv6_flag = "-6";
    uint32_t ipv6_flag_index_location = 1;

    if (arg_count < 2) {
      std::cerr
          << "Usage: ./sender [-f data_file (Optional)] [-6 (Optional)] [port] "
          << std::endl;
      std::exit(1);
    }

    if (args[1] == file_flag) {
      file_path = args[2];
      if (!fs::exists(file_path)) {
        throw std::runtime_error("Error, file (path) not found");
      }
    }

    if (args[ipv6_flag_index_location] == ipv6_flag) {
      ip_version = IPVersion::IPv6;
      port = atoi(args[ipv6_flag_index_location + 1]);
    } else {
      ip_version = IPVersion::IPv4;
      port = atoi(args[1]);
    }
  }
};

enum SegmentType {
  PTYPE_DATA = 1,
  PTYPE_ACK = 2,
  PTYPE_NACK = 3,
};

void ipv4UDP(const char *hostname, int port, fs::path file) {}

void ipv6UDP(const char *hostname, int port, fs::path file) {}
int main(int argc, char *argv[]) {}
