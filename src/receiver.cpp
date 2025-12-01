#include "../include/PacketHeader.h"
#include <arpa/inet.h>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <netinet/in.h>
#include <stdexcept>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <zlib.h>
#include <fstream>
#include <sstream>
#include <cstring>

namespace fs = std::filesystem;

// TODO:
// - Test initial code, check if can communicate w/ receiver on lab computers
// - Talk with robert on how the data file should be set up w/ ReceiverArgs
// ---> should the command line create said file if it does not exist
// ---> should the program only work if said file exists
// - Update packet header to take out the payload
// - Find out how to check crc on code
//
//
// Additional Resources (aside from beej)
// - https://www.yolinux.com/TUTORIALS/Sockets.html#DESCRIPTION
// -
// https://users.cs.jmu.edu/bernstdh/web/common/lectures/summary_unix_udp.php?utm_source=chatgpt.com
// - https://www.tenouk.com/cnlinuxsockettutorials.html?utm_source=chatgpt.com
// - https://www.educative.io/answers/how-to-implement-udp-sockets-in-c
using BYTES = unsigned char *; // our program slightly more readable

static constexpr uint32_t BUFFER_SIZE = 4096;
static constexpr uint32_t CRC_1_BUFFER_INDEX = 8;
static constexpr uint32_t PAYLOAD_START_INDEX = 12;
static constexpr uint32_t BUFFER_META_DATA_SIZE = 16;

enum IPVersion { IPv4, IPv6 };

struct ReceiverArgs {
  IPVersion ip_version;
  fs::path file_path = "";
  uint32_t port_number;

  ReceiverArgs(int arg_count, char *args[]) {
    //std::string file_flag = "-f"; //We always want a file
    std::string ipv6_flag = "-6";
    uint32_t ipv6_flag_index_location = 3;

    if (arg_count < 2) {
      std::cerr
          << "Usage: ./receiver [-f data_file] [-6 (IPv6:Optional)] [port] "
          << std::endl;
      std::exit(1);
    }

    //Always want a file
    file_path = args[2];
    ipv6_flag_index_location = 3;
    if (!fs::exists(file_path)) {
      throw std::runtime_error("Error, file (path) not found");
    }

    //ipv6 Currently Broken
    if (args[ipv6_flag_index_location] == ipv6_flag) {
      ip_version = IPVersion::IPv6;
      port_number = atoi(args[ipv6_flag_index_location + 1]);
    } else {
      ip_version = IPVersion::IPv4;
      port_number = atoi(args[3]);
    }
  }

  void display() const {
    std::cout << "IP_VERSION: " << this->ip_version << '\n'
              << "PORT: " << this->port_number << '\n'
              << "FILEPATH: " << this->file_path << std::endl;
  }
};

enum SegmentType {
  PTYPE_DATA = 1,
  PTYPE_ACK = 2,
  PTYPE_NACK = 3,
};

void report_error(const char *error_msg) {
  perror(error_msg);
  exit(EXIT_FAILURE);
}

void dead_factory(BYTES buffer, int seq) {
  memset(buffer, 0, BUFFER_META_DATA_SIZE);

  PacketHeader header;
  header.setType(buffer, SegmentType::PTYPE_ACK);
  header.setTR(buffer, false);
  header.setLen(buffer, 0);
  header.setSeq(buffer, seq);
  header.setTimeStamp(buffer, 4022250974); //This should be pulled in from previous packet, not a const
  header.setWin(buffer, 31);

  // Compute CRC1 over first 8 bytes (start of type ---> end of Timestamp)
  uint32_t crc1 = crc32(0L, Z_NULL, 0);
  crc1 = crc32(crc1, buffer, CRC_1_BUFFER_INDEX);
  header.setCRC(buffer, crc1); // Store in little-endian
}

void ipv4UDP(int port_number, fs::path file) {
  char buffer[BUFFER_SIZE];
  int socket_handle;
  int seq = 1;
  if ((socket_handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    report_error("ERROR, socket creation failed");

  struct sockaddr_in server_address{}, client_address{};
  socklen_t client_length = sizeof(client_address);

  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port_number);
  server_address.sin_addr.s_addr = INADDR_ANY;

  if (bind(socket_handle, (const struct sockaddr *)&server_address,
           sizeof(server_address)) < 0) {
    close(socket_handle);
    report_error("ERROR, socket binding failed");
  }

while (true) {
    socklen_t client_length = sizeof(client_address);
    int client_data = recvfrom(socket_handle, buffer, BUFFER_SIZE, MSG_WAITALL,
                               (struct sockaddr*)&client_address, &client_length);

    if (client_data < 0) {
        std::cerr << "recvfrom failed\n";
        continue;  // Skip to next iteration instead of breaking
    }

    if (client_data > 0) {

        // Print incoming bytes as binary
        for (int i = 0; i < client_data; i++) {
            for (int bit = 7; bit >= 0; bit--) {
                std::cout << ((buffer[i] >> bit) & 1);
            }
            std::cout << " ";
        }
        std::cout << std::endl;

        // Append instead of overwrite (use std::ios::app)
        std::ofstream out(file, std::ios::out | std::ios::app);
        if (!out.is_open()) {
            throw std::runtime_error("Failed to open file: " + file.string());
        }

        // Write the same binary data to file
        for (int i = 0; i < client_data; i++) {
            for (int bit = 7; bit >= 0; bit--) {
                out << ((buffer[i] >> bit) & 1);
            }
            out << " ";
        }
        out << std::endl;
    }
  //Data Auditng needs to be done here now
  //If its a bad CRC1, or CRC2 we need to drop the packet!!!
  //TODO - CRC1 and 2 Checks, and/or drop packet - med
  //     - Translate over the timestamp to the dead packet function - easy
  //        Implemented a constant instead
  //     - Fix IPv6 it doesnt even work at all - ugh
  uint32_t dead_buffer_size = BUFFER_META_DATA_SIZE;
  unsigned char *deadbuffer = new unsigned char[dead_buffer_size];
  dead_factory(deadbuffer,seq);
  seq = seq+1;
  sendto(socket_handle, (const char *)deadbuffer, dead_buffer_size, 0,
         (const struct sockaddr *)&client_address, sizeof(client_address));
}

  //Send acknowledgement packet as well before closing
  close(socket_handle);
}

void ipv6UDP(int port_number, fs::path file) {
  char buffer[BUFFER_SIZE];
  int socket_handle;

  if ((socket_handle = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP)) < 0)
    report_error("ERROR, socket creation failed");

  struct sockaddr_in server_address{}, client_address{};
  socklen_t client_length = sizeof(client_address);

  server_address.sin_family = AF_INET6;
  server_address.sin_port = htons(port_number);
  server_address.sin_addr.s_addr = INADDR_ANY;

  if (bind(socket_handle, (const struct sockaddr *)&server_address,
           sizeof(server_address)) < 0) {
    close(socket_handle);
    report_error("ERROR, socket binding failed");
  }

  int client_data =
      recvfrom(socket_handle, buffer, BUFFER_SIZE, MSG_WAITALL,
               (struct sockaddr *)&client_address, &client_length);


if (client_data > 0) {
    for (int i = 0; i < client_data; i++) {
        std::cout << std::hex << std::uppercase
                  << static_cast<int>(static_cast<unsigned char>(buffer[i])) << " ";
    }
    std::cout << std::dec << std::endl; // restore decimal
    std::ofstream out(file, std::ios::out);
    if (!out.is_open()) {
        throw std::runtime_error("Failed to open file: " + file.string());
    }

    for (int i = 0; i < client_data; i++) {
        out << std::hex << std::uppercase
            << static_cast<int>(buffer[i]) << " ";
    }
    out << std::dec << std::endl; // reset formatting
  }
  //Data Auditng needs to be done here now
  close(socket_handle);
}

int main(int argc, char *argv[]) {
  ReceiverArgs receiver_args = ReceiverArgs(argc, argv);
  receiver_args.display();

  if (receiver_args.ip_version == IPVersion::IPv4) {
    ipv4UDP(receiver_args.port_number, receiver_args.file_path);
  } else {
    ipv6UDP(receiver_args.port_number, receiver_args.file_path);
  }
}
