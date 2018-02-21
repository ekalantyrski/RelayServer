//
// Created by Eric Kalantyrski on 2018-02-04.
//

//
//  server.cpp
//  RelayServer
//
//  Created by Eric Kalantyrski on 2018-01-11.
//

#include "server.h"


struct connectionInfo {
    int socket;
    bool isPhone;
    std::string address;
};

void processConnection(int socket, sockaddr_in clientAddr, std::map<std::string, connectionInfo> &pwToConnection)
{
    char buffer[256] = {};
    int readValue = read(socket, buffer, 256);
    std::string input(buffer);
    //reset buffer
    memset(&buffer, 0, sizeof(buffer));
    /*
     The input will a 10 char password, localip and a 1 or 0
     signifying if the connection is the phone or computer.
     They will be seperated by a space
     The 1 indicates that it is the phone that connected, while 0
     indicates it was the computer.
     */
    std::istringstream iss(input);
    std::vector<std::string> results((std::istream_iterator<std::string>(iss)),
                                     std::istream_iterator<std::string>());



    bool isPhone = (results[2].at(0) == '1');
    // get just the address, (substring the whole input, minus the last character)
    std::string address(results[1]);
    std::string password(results[0]);

    std::cout << address << " " << isPhone << " " << password << "\n";

    // check if password is already mapped
    int count = pwToConnection.count(password);
    if(count > 0)
    {
        // if address is mapped, check if the mapped address if a phone or computer
        connectionInfo storedInfo = pwToConnection[password];
        std::string addressToSend;
        int socketToSend;
        if(isPhone) // if current connection is the phone
        {
            addressToSend = storedInfo.address;
            socketToSend = socket;

        }
        else
        {
            //then current connection is the computer
            addressToSend = address;
            socketToSend = storedInfo.socket;
            std::cout << "Second connection processed.\n";
        }

        send(socketToSend, addressToSend.c_str(), addressToSend.size(), 0);
        // once data is sent, delete connection from map
        std::map<std::string, connectionInfo>::iterator it;
        it = pwToConnection.find(password);
        pwToConnection.erase(it);

        int closeValue = close(storedInfo.socket);
        if(closeValue < 0)
        {
            perror("failed to close stored client socket");
        }

        closeValue = close(socket);
        if(closeValue < 0)
        {
            perror("failed to close given client socket");
        }

#warning deconstruct data after use
    }
    else
    {
        // if password not mapped, map the given connectionInfo
        connectionInfo newInfo;
        newInfo.isPhone = isPhone;
        newInfo.address = address;
        newInfo.socket = socket;

        pwToConnection[password] = newInfo;
        std::cout << "First connection processed.\n";
    }
}

int main()
{
    in_port_t PORT = PORT_NUMBER;
    std::cout << PORT;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t cli_len;
    // create a map to map passwords to connections
    std::map<std::string, connectionInfo> pwToConnection;

    //create socket to accept connections
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    //bzero((char *) &serverAddr, sizeof(serverAddr));
    memset((char *) &serverAddr, 0, sizeof(serverAddr));
    //bind socket
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    int bindResult = bind(sockfd, (struct sockaddr *) &serverAddr,   sizeof(serverAddr));
    if(bindResult < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    // set socket to listen
    std::cout << "Bound\n";
    listen(sockfd, 5);


    cli_len = sizeof(clientAddr);
    // accept a connection
    while(1)
    {
        int clientSocket = accept(sockfd, (struct sockaddr *) &clientAddr, &cli_len);
        int start = clock();
        if(clientSocket < 0)
        {
            perror("accept failed");
        }
        std::cout << "Accepted\n";
        // create a thread to process connection
        processConnection(clientSocket, clientAddr, pwToConnection);
        int stop = clock();
        std::cout << "Time to execute: " <<  ((stop-start)/double(CLOCKS_PER_SEC)*1000) << std::endl;
    }

    int sdValue = shutdown(sockfd, SHUT_RDWR);
    if(sdValue < 0)
    {
        perror("Shutdown failed");
    }
    int closeValue = close(sockfd);
    if(closeValue < 0)
    {
        perror("Close failed");
    }

    std::cout << "Closed successfully\n";



}





