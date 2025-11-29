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
      port_number = atoi(args[ipv6_flag_index_location + 1]);
    } else {
      ip_version = IPVersion::IPv4;
      port_number = atoi(args[1]);
    }
  }
};

enum SegmentType {
  PTYPE_DATA = 1,
  PTYPE_ACK = 2,
  PTYPE_NACK = 3,
};

void ipv4UDP(int port_number, fs::path file) {
  char buffer[BUFFER_SIZE];
  int socket_handle;

  // Think IPPROTO_IP macro / const = 0, if system does not work replace w/ 0
  if ((socket_handle = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP) < 0)) {
    close(socket_handle);
    perror("ERROR, socket creation failed");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in server_address, client_address;
  memset(&server_address, 0, sizeof(server_address));
  memset(&client_address, 0, sizeof(client_address));

  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port_number);
  server_address.sin_addr.s_addr = INADDR_ANY;

  if (bind(socket_handle, (const struct sockaddr *)&server_address,
           sizeof(server_address)) < 0) {
    close(socket_handle);
    perror("ERROR, bind failed");
    exit(EXIT_FAILURE);
  }

  listen(socket_handle, 5);

  socklen_t client_length = sizeof(client_address);

  int client_data =
      recvfrom(socket_handle, (char *)buffer, BUFFER_SIZE, MSG_WAITALL,
               (struct sockaddr *)&client_address, &client_length);

  std::cout << client_data << std::endl; // test data is received

  close(socket_handle);
}

void ipv6UDP(int port_number, fs::path file) {
  char buffer[BUFFER_SIZE];
  int socket_handle;

  // Think IPPROTO_IP macro / const = 0, if system does not work replace w/ 0
  if ((socket_handle = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP) < 0)) {
    close(socket_handle);
    perror("ERROR, socket creation failed");
    exit(EXIT_FAILURE);
  }

  struct sockaddr_in server_address, client_address;
  memset(&server_address, 0, sizeof(server_address));
  memset(&client_address, 0, sizeof(client_address));

  server_address.sin_family = AF_INET6;
  server_address.sin_port = htons(port_number);
  server_address.sin_addr.s_addr = INADDR_ANY;

  if (bind(socket_handle, (const struct sockaddr *)&server_address,
           sizeof(server_address)) < 0) {
    close(socket_handle);
    perror("ERROR, bind failed");
    exit(EXIT_FAILURE);
  }

  socklen_t client_length = sizeof(client_address);

  int client_data =
      recvfrom(socket_handle, (char *)buffer, BUFFER_SIZE, MSG_WAITALL,
               (struct sockaddr *)&client_address, &client_length);

  std::cout << client_data << std::endl; // test data is received

  close(socket_handle);
}

int main(int argc, char *argv[]) {
  ReceiverArgs receiver_args = ReceiverArgs(argc, argv);

  if (receiver_args.ip_version == IPVersion::IPv4) {
    ipv4UDP(receiver_args.port_number, receiver_args.file_path);
  } else {
    ipv6UDP(receiver_args.port_number, receiver_args.file_path);
  }
}
