#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define MAX_CLIENTS 10

void handle_client(int client_socket) {
    // Login logic, handling mode 1 and mode 2
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <Port>\n", argv[0]);
        exit(1);
    }

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_size = sizeof(client_addr);
    int port = atoi(argv[1]);

    // Create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    // Bind to port
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        exit(1);
    }

    // Listen for incoming connections
    listen(server_socket, MAX_CLIENTS);

    printf("Server listening on port %d\n", port);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);
        if (client_socket < 0) {
            perror("Client connection failed");
            continue;
        }

        // Handle each client in a separate process or thread
        handle_client(client_socket);
        close(client_socket);
    }

    close(server_socket);
    return 0;
}
