// Server side implementation of UDP client-server model 
#include <bits/stdc++.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
  
#define PORT     12345
#define MAXLINE 4096 //Max ammount of bits


void writeToFile(const char* buffer) {
        //Write into Binary file
    std::ofstream outFile("ReceiverFile.bin", std::ios::binary);
    if (!outFile) {
        std::cerr << "Error opening file for writing!" << std::endl;
    }
    // Write the buffer to the file, including the null terminator
    outFile.write(buffer, strlen(buffer));
    // Close the file
    outFile.close();
    std::cout << "Data written to ReceiverFile.bin" << std::endl;
}

//IPv4 : UDP - working
// Driver code 
void ipv4UDP() {
    int sockfd; 
    char buffer[MAXLINE]; 
    struct sockaddr_in servaddr, cliaddr; 

    //Will need these as inputs next, address and port
      
    // Creating socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 

    //Clearing Memory
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
    // Filling server information 
    servaddr.sin_family = AF_INET; // IPv4 
    //Note: If below is exempted, default appears to be ANY IP address
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT); 
      
    // Bind the socket with the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    }
    
    std::cout << "Server is listening for incoming connections..." << std::endl;
    socklen_t len;
    int n; 
    len = sizeof(cliaddr);  //len is value/result 
  
    //Await for a packet
    n = recvfrom(sockfd, (char *)buffer, MAXLINE,  
                MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
                &len); 
    buffer[n] = '\0'; 
    printf("Client : %s\n", buffer); 
    writeToFile(buffer);
}
void ipv6UDP() {
        int sockfd; 
    char buffer[MAXLINE]; 
    struct sockaddr_in6 servaddr, cliaddr; 

    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) { 
        perror("socket creation failed");
        exit(EXIT_FAILURE); 
    }

    // Clearing memory
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Filling server information for IPv6
    servaddr.sin6_family = AF_INET6; // IPv6
    servaddr.sin6_port = htons(PORT);

    // Use the loopback address for IPv6 (::1)
    if (inet_pton(AF_INET6, "::1", &servaddr.sin6_addr) <= 0) {
        perror("Invalid IPv6 address");
        exit(EXIT_FAILURE);
    }

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE); 
    }

    socklen_t len;
    int n;
    len = sizeof(cliaddr);  // Length of client address
    std::cout << "Server is listening for incoming connections..." << std::endl;

    // Await for a packet
    n = recvfrom(sockfd, (char *)buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *) &cliaddr, &len);
    buffer[n] = '\0'; // Null-terminate the received data

    std::cout << "Client (IPv6) : " << buffer << std::endl;

    // Write the received data to a file
    writeToFile(buffer);

    // Optionally, you can send a response to the client:
    const char *response = "Data received";
    sendto(sockfd, response, strlen(response), 0, (const struct sockaddr *) &cliaddr, len);

    // Close the socket
    close(sockfd);
}
/*
void ipv4TCP() {
    int sockfd, newsockfd;
    char buffer[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    //Note: If below is exempted, default appears to be ANY IP address
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    servaddr.sin_port = htons(PORT);

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server is listening for incoming connections..." << std::endl;

    if (listen(sockfd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    len = sizeof(cliaddr);
    newsockfd = accept(sockfd, (struct sockaddr *)&cliaddr, &len);
    if (newsockfd < 0) {
        perror("Server accept failed");
        exit(EXIT_FAILURE);
    }

    // Receive the data
    int bytes_received = recv(newsockfd, buffer, MAXLINE, 0);
    if (bytes_received < 0) {
        perror("Receive failed");
    } else {
        buffer[bytes_received] = '\0'; // Null-terminate for safe80::3617:ebff:fec3:438fe printing
        writeToFile(buffer);
        std::cout << "Received message: " << buffer << std::endl;
        
        // Send acknowledgment back to client
        const char* ack = "Data received successfully";
        send(newsockfd, ack, strlen(ack), 0);
    }

    close(newsockfd);
    close(sockfd);
}
*/

/* Tcp was not needed
void ipv6TCP() {
    int sockfd, newsockfd;
    struct sockaddr_in6 servaddr, cliaddr;
    char buffer[MAXLINE];

    // Create socket for IPv6 TCP
    if ((sockfd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Clear the server address structure
    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // Setup server information for IPv6
    servaddr.sin6_family = AF_INET6;  // IPv6
    servaddr.sin6_port = htons(PORT);  // Port to listen on

    // Use the loopback address for IPv6 (::1)
    if (inet_pton(AF_INET6, "::1", &servaddr.sin6_addr) <= 0) {
        perror("Invalid IPv6 address");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the address
    if (bind(sockfd, (const struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(sockfd, 5) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is listening for incoming connections..." << std::endl;
    socklen_t len = sizeof(cliaddr);
    
    // Accept an incoming connection
    if ((newsockfd = accept(sockfd, (struct sockaddr*)&cliaddr, &len)) < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    // Receive data from the sender
    int n = recv(newsockfd, buffer, MAXLINE, 0);
    if (n < 0) {
        perror("Receive failed");
    } else {
        buffer[n] = '\0';  // Null-terminate the received data
        std::cout << "Received " << n << " bytes" << std::endl;
        std::cout << "Data: " << buffer << std::endl;

        // Write the received data to a file
        writeToFile(buffer);
    }

    // Close the sockets
    close(newsockfd);
    close(sockfd);
}
*/

int main() { 
    //Need to test, port mismatches, ip mistmatches, protocol mismatches
    //TODO - Get cmd inline arguments for port and IP
    //Sender Example: ./sender receiver_address 3000(Port)
    //Once this is inplace, We need to 
    
    ipv4UDP(); // Works(Passes All 3)
    //ipv6UDP(); //Works(port,protocol) Needs ip Testing

    //ipv4TCP(); // Works(Passed All 3) - Extra
    //ipv6TCP(); //Works(port,protocol) - Extra
      
    return 0; 
}