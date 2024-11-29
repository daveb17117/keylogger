#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib") // Link the Winsock library

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;
    int client_size;
    char buffer[1024] = {0};

    // Initialize Winsock
    printf("\nInitializing Winsock...");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Failed. Error Code: %d\n", WSAGetLastError());
        return 1;
    }
    printf("Initialized.\n");

    // Create a socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("Socket created.\n");

    // Prepare the sockaddr_in structure
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY; // Accept connections on any IP
    server.sin_port = htons(443);

    // Bind
    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Bind failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
    printf("Bind done.\n");

    // Listen to incoming connections
    listen(server_socket, 3);
    printf("Waiting for incoming connections...\n");

    // Accept an incoming connection
    client_size = sizeof(struct sockaddr_in);
    client_socket = accept(server_socket, (struct sockaddr *)&client, &client_size);
    if (client_socket == INVALID_SOCKET) {
        printf("Accept failed. Error Code: %d\n", WSAGetLastError());
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }
    printf("Connection accepted.\n");

    while (1) {
        int recv_size = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
        if (recv_size == SOCKET_ERROR) {
            printf("Recv failed. Error Code: %d\n", WSAGetLastError());
            break;
        }
        if (recv_size == 0) { // Connection closed by client
            printf("Client disconnected.\n");
            break;
        }

        buffer[recv_size] = '\0'; // Null-terminate the received data
        printf("Received data: %s\n", buffer);
    }

    // Cleanup
    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();

    return 0;
}
