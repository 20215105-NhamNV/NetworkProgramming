#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void send_string(int client_sock)
{
    char input[BUFFER_SIZE];
    while (1)
    {
        printf("Enter a string (empty string to exit): ");
        fgets(input, BUFFER_SIZE, stdin);
        if (input[0] == '\n')
            break;
        int bytes_sent = send(client_sock, input, strlen(input), 0); // send data from client to server
        if (bytes_sent < 0)
        {
            printf("error send data to server\n");
            close(client_sock);
        }
    }
}

void send_file(int client_sock)
{
    char filename[BUFFER_SIZE];
    printf("Enter a filename: ");
    scanf("%s", filename);
    FILE *file = fopen(filename, "r");
    if (!file)
    {
        printf("File open failed");
        return;
    }
    char buffer[BUFFER_SIZE] = "FILE:";
    fread(buffer + 5, 1, BUFFER_SIZE - 5, file);
    fclose(file);
    send(client_sock, buffer, strlen(buffer), 0);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Invalid command Line Argument!\n");
        return 0;
    }

    char *SERV_IP = argv[1];
    int SERV_PORT = atoi(argv[2]); // server port
    int client_sock;
    struct sockaddr_in server_addr;

    // Construct a TCP socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (client_sock < 0)
    {
        printf("error create socket\n");
        return 0;
    }

    // Define the address of the server
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERV_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERV_IP);

    // connect to server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Connection failed!");
        close(client_sock);
        return 0;
    }

    printf("MENU\n-------------------------\n1. Send a string\n2. Send a file\n");

    int choice;
    scanf("%d", &choice);
    getchar();
    if (choice == 1)
    {
        send_string(client_sock);
    }
    else if (choice == 2)
    {
        send_file(client_sock);
    }
    close(client_sock);
    return 0;
}
