#include <bits/stdc++.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include "../include/PacketHeader.h"
#include <zlib.h>

typedef unsigned char BYTE;
#define MAXLINE 4096 

void factoryEnd(unsigned char *buffer) {
    memset(buffer, 0, 16); // Clear buffer

    PacketHeader header;

    header.setType(buffer, 1);     // Type = 1 (PTYPE_DATA)
    header.setTR(buffer, false);   // TR = 0 (not truncated)
    header.setWin(buffer, 0);      // Window = 0
    header.setSeq(buffer, 0);      // Ending frame seq = 1
    header.setLen(buffer, 0);      // Length = 0 (no payload)

    // Timestamp (4 bytes)
    buffer[4] = buffer[5] = buffer[6] = buffer[7] = 0;

    // CRC1 placeholder
    buffer[8] = buffer[9] = buffer[10] = buffer[11] = 0;

    // Compute CRC1 over first 12 bytes
    uint32_t crc1 = crc32(0L, Z_NULL, 0);
    crc1 = crc32(crc1, buffer, 12);

    header.setCRC(buffer, crc1);

    // CRC2 placeholder (empty payload)
    uint32_t crc2 = crc32(0L, Z_NULL, 0);

    // Store CRC2 in big-endian order
    buffer[12] = (crc2 >> 24) & 0xFF;
    buffer[13] = (crc2 >> 16) & 0xFF;
    buffer[14] = (crc2 >> 8) & 0xFF;
    buffer[15] = crc2 & 0xFF;
}

void factoryPayload(unsigned char *buffer, const char *payload, size_t payload_len) {
    memset(buffer, 0, 16 + payload_len); // Clear buffer + space for payload

    PacketHeader *header = new PacketHeader();

    // Header fields
    header->setType(buffer, 1);       // Type = 1 (PTYPE_DATA)
    header->setTR(buffer, false);     // TR = 0
    header->setWin(buffer, 0);        // Window = 0
    header->setSeq(buffer, 0);        // Sequence number = 0
    header->setLen(buffer, static_cast<uint>(payload_len)); // Length = payload length

    // Timestamp (4 bytes)
    buffer[4] = buffer[5] = buffer[6] = buffer[7] = 0;

    // CRC1 placeholder (zeroed)
    buffer[8] = buffer[9] = buffer[10] = buffer[11] = 0;

    // Compute CRC1 over first 12 bytes
    uint32_t crc1 = crc32(0L, Z_NULL, 0);
    crc1 = crc32(crc1, buffer, 12);
    header->setCRC(buffer, crc1); // Store in little-endian

    // Copy payload immediately after header + CRC1
    if (payload_len > 0) {
        memcpy(buffer + 12, payload, payload_len);
    }

    // Compute CRC2 over the payload
    uint32_t crc2 = crc32(0L, Z_NULL, 0);
    if (payload_len > 0) {
        crc2 = crc32(crc2, reinterpret_cast<const unsigned char*>(payload), payload_len);
    }

    // Store CRC2 after header + payload in little-endian
    size_t crc2_offset = 12 + payload_len;
    buffer[crc2_offset]     = crc2 & 0xFF;
    buffer[crc2_offset + 1] = (crc2 >> 8) & 0xFF;
    buffer[crc2_offset + 2] = (crc2 >> 16) & 0xFF;
    buffer[crc2_offset + 3] = (crc2 >> 24) & 0xFF;

    delete header;
}

std::vector<BYTE> readFile(const char* filename)
{
    // open the file:
    std::ifstream file(filename, std::ios::binary);

    // get its size:
    file.seekg(0, std::ios::end);
    std::streampos fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // read the data:
    std::vector<BYTE> fileData(fileSize);
    file.read((char*) &fileData[0], fileSize);
    return fileData;
}

// Driver code 
void ipv4UDP(const char* hostname, int port, const char* file) {
    int sockfd; 

    //optimization be just read the x bytes instead of all of them
    //Read from binary file
    std::vector<BYTE> payload = readFile(file); //This needs to be file
    // Read binary file into vector

     // Allocate buffer for header + payload + CRC2
    size_t buffer_size = 16 + payload.size(); // 16 bytes header + timestamp + CRC2
    unsigned char* buffer = new unsigned char[buffer_size];

    // Call factoryPayload with pointer to buffer and payload
    factoryPayload(buffer, reinterpret_cast<const char*>(payload.data()), payload.size());

    struct sockaddr_in servaddr; 
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    //Clearing memory
    memset(&servaddr, 0, sizeof(servaddr)); 
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(hostname); //This needs to be host name

    int n;
    socklen_t len; 

    //Sending Packet
    sendto(sockfd, (const char *)buffer, payload.size(), 
        MSG_CONFIRM, (const struct sockaddr *) &servaddr,  
            sizeof(servaddr)); 


    //Data Prep for empty
    //char buffer[MAXLINE]; 
    // Clear buffer 
    //memset(buffer, 0, MAXLINE);
    //We will need to send payload first here then Ending file
    //factoryEnd((unsigned char*)buffer); // Ending File
    //TODO SEND EMPTY PACKET TO END CONNECTION

    close(sockfd); 
}
void ipv6UDP(const char* hostname, int port, const char* file){
    int sockfd;
    char buffer[MAXLINE];

    std::vector<BYTE> fileData = readFile(file); //This needs to be file
    std::string temp;
    for (int i = 0; i < fileData.size(); ++i) // read bits into temp
        temp.push_back(fileData[i]);
    const char *payload = temp.c_str();

    struct sockaddr_in6 servaddr;
    // Creating socket file descriptor for IPv6
    if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Clearing memory
    memset(&servaddr, 0, sizeof(servaddr));

    // Filling server information for IPv6
    servaddr.sin6_family = AF_INET6; // IPv6
    servaddr.sin6_port = htons(port);
    
    // Use the loopback address for IPv6 (::1)
    if (inet_pton(AF_INET6, hostname, &servaddr.sin6_addr) <= 0) {
        perror("Invalid IPv6 address");
        exit(EXIT_FAILURE);
    }

    //Send a Packet
    //TODO WE NEED TO USE PA1 here, append it all
    //Then send it
    int bytes_sent = sendto(sockfd, (const char *)payload, fileData.size(), MSG_CONFIRM, 
                            (const struct sockaddr *) &servaddr, sizeof(servaddr));
    
    if (bytes_sent < 0) {
        perror("Send failed");
    } else {
            std::cout<< "Msg sent:"<<payload <<std::endl; 
    }

    close(sockfd);
}


//Expecting order of receiver
//With the data
//empty data and sequence number 1 is the end of tranmission

//Means we need PA1
//PA1 needs a payload section added
//PA1 needs a way 

int main(int argc, char* argv[]) { 
    if (argc < 3) {
        std::cerr << "Usage: ./sender [hostname] [port] [-f filename]" << std::endl;
        return 1;
    }

    const char* hostname = argv[1];
    int port = std::stoi(argv[2]);
    const char* filename = "";

    // Parse optional -f flag and filename
    for (int i = 3; i < argc; i++) {
        if (std::string(argv[i]) == "-f") {
            if (i + 1 < argc) {
                filename = argv[i + 1];  // Set filename to the next argument
                i++;  // Skip next argument since it's already used as filename
            } else {
                std::cerr << "Error: Missing filename after -f flag." << std::endl;
                return 1;
            }
        }
    }
    std::cout << "hostname: " << hostname << std::endl;
    std::cout << "PORT: " << port << std::endl;
    std::cout << "filename: " << filename << std::endl;

    //TODO - detect which one to run
    //And switch to that option
    ipv4UDP(hostname,port,filename); 
    //ipv6UDP(hostname,port,filename);


    return 0; 
}