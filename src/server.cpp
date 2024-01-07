/**
 * Server side of one-to-one chat
 * 
 * Listens for a connection with 1 client
 * Send and receive messages with the client
 *   Receiving: Acknowledge received messages by sending "message received"
 *   Sending: Calculate RTT by waiting for acknowledgement message from client
 * When the client terminates connection, continue listening for other clients
*/

#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

void sendMessage(int clientSocket)
{
    char buf[4096];
    std::string userInput;

    while (true)
    {
        // Enter lines of text
        std::cout << "> ";
        getline(std::cin, userInput);
        
        // Send to client
        auto begin = std::chrono::high_resolution_clock::now();
        int sendRes = send(clientSocket, userInput.c_str(), userInput.size()+1, 0);
        if (sendRes == -1)
        {
            std::cout << "Could not send to client!\n";
            break;
        }
        // Wait for response
        memset(buf, 0, 4096);
        int bytesReceived = recv(clientSocket, buf, 4096, 0);
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
        if (bytesReceived == -1)
        {
            std::cout << "There was an error getting response from client\r\n";
        }
        else if (bytesReceived == 0)
        {
            break;
        }
        else
        {
            // Display response
            std::cout << "CLIENT> " << std::string(buf, bytesReceived) << "\r\n";
            std::cout << "RTT (in nanoseconds): " << elapsed.count() << "\n\n";
        }
    };

    return;
}

void receiveMessage(int clientSocket) 
{
    const char* const ACK = "message received";
    const int ACK_LENGTH = 17;
    char buf[4096];
    while (true)
    {
        // Clear the buffer
        memset(buf, 0, 4096);

        // Wait for a message
        int bytesRecv = recv(clientSocket, buf, 4096, 0);
        if (bytesRecv == -1)
        {
            std::cerr << "There was a connection issue\n";
            break;
        }

        if (bytesRecv == 0)
        {
            std::cout << "The client disconnected\n";
            break;
        }

        // Send ACK
        send(clientSocket, ACK, ACK_LENGTH, 0);
    }
    close(clientSocket);
    return;
}


int main()
{
    // Create a socket
    int listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == -1)
    {
        std::cerr << "Can't create a socket!\n";
        return 1;
    }

    // Bind the socket to a IP / port
    int port = 54000;
    std::string ipAddress = "0.0.0.0";

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    if (bind(listening, (sockaddr*)&hint, sizeof(hint)) == -1)
    {
        std::cerr << "Can't bind to IP/port!\n";
        return 1;
    }

    // Mark the socket for listening in
    if (listen(listening, SOMAXCONN) == -1)
    {
        std::cerr << "Can't listen!\n";
        return 1;
    }

    // Accept a call
    sockaddr_in client;
    socklen_t clientSize = sizeof(client);
    char host[NI_MAXHOST];
    char service[NI_MAXSERV];

    int clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
    while (clientSocket != -1)
    {
        memset(host, 0, NI_MAXHOST);
        memset(service, 0, NI_MAXSERV);

        int result = getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0);
        if (result)
        {
            std::cout << host << " connected on " << service << std::endl;
        }
        else
        {
            inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
            std::cout << host << " connected on " << ntohs(client.sin_port) << std::endl;
        }

        // Send and Receive messages
        std::thread sendThread(sendMessage, clientSocket);
        std::thread receiveThread(receiveMessage, clientSocket);

        receiveThread.join();
        sendThread.join();

        // Client disconnect, continue listening for connections
        clientSocket = accept(listening, (sockaddr*)&client, &clientSize);
    }

    if (clientSocket == -1)
    {
        std::cerr << "Problem with client connecting!\n";
        return 1;
    }

    // Close the listening socket
    close(listening);

    return 0;
}