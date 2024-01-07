/**
 * Client side of one-to-one chat
 * 
 * Takes in an IP address or hostname to connect to
 * Send and receive messages with the server
 *   Receiving: Acknowledge received messages by sending "message received"
 *   Sending: Calculate RTT by waiting for acknowledgement message from server
*/

#include <arpa/inet.h>
#include <chrono>
#include <iostream>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

void sendMessage(int sock)
{
    char buf[4096];
    std::string userInput;

    while (true)
    {
        // Enter lines of text
        std::cout << "> ";
        getline(std::cin, userInput);
        
        // Send to server
        auto begin = std::chrono::high_resolution_clock::now();
        int sendRes = send(sock, userInput.c_str(), userInput.size()+1, 0);
        if (sendRes == -1)
        {
            std::cout << "Could not send to server!\n";
            continue;
        }
        // Wait for response
        memset(buf, 0, 4096);
        int bytesReceived = recv(sock, buf, 4096, 0);
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
        if (bytesReceived == -1)
        {
            std::cout << "There was an error getting response from server\r\n";
        }
        else if (bytesReceived == 0)
        {
            break;
        }
        else
        {
            // Display response
            std::cout << "SERVER> " << std::string(buf, bytesReceived) << "\r\n";
            std::cout << "RTT (in nanoseconds): " << elapsed.count() << "\n\n";
        }
    };

    // Close the socket
    close(sock);
    return;   
}

void receiveMessage(int sock) 
{
    const char* const ACK = "message received";
    const int ACK_LENGTH = 17;
    char buf[4096];
    while (true)
    {
        // Clear the buffer
        memset(buf, 0, 4096);

        // Wait for a message
        int bytesRecv = recv(sock, buf, 4096, 0);
        if (bytesRecv == -1)
        {
            std::cerr << "There was a connection issue\n";
            break;
        }

        if (bytesRecv == 0)
        {
            std::cout << "The server disconnected\n";
            break;
        }

        // Send ACK
        send(sock, ACK, ACK_LENGTH, 0);
    }

    return;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: ip_address\n";
        return 1;
    }
    char *serverIp= argv[1];
    // Create a socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        std::cerr << "Can't create a socket!\n";
        return 1;
    }

    // Create a hint structure for the server we're connecting with
    int port = 54000;
    struct hostent* host = gethostbyname(serverIp);

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    hint.sin_addr.s_addr = inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
    // inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    // Connect to the server on the socket
    int connectRes = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connectRes == -1)
    {
        std::cerr << "Can't connect to server. Make sure the server is up\n";
        return 1;
    }
    std::thread sendThread(sendMessage, sock);
    std::thread receiveThread(receiveMessage, sock);

    receiveThread.join();
    sendThread.join();
    
    return 0;
}