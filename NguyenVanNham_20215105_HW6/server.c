#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "sha256.h"
#include <ctype.h>

#define BUFFER_SIZE 1024

int is_valid_string(const char *buffer)
{
    int i = 0;
    while (buffer[i] != '\0') // Lặp qua từng ký tự trong buffer
    {
        if (!isalpha(buffer[i]) && !isdigit(buffer[i]) && buffer[i] != '\0') // Nếu không phải chữ cái hoặc chữ số
        {
            return 0; // Chuỗi không hợp lệ
        }
        i++;
    }
    return 1; // Chuỗi hợp lệ
}

void handle_string(const char *input, char *letter, char *digit)
{
    int i = 0, j = 0, k = 0;
    while (input[i])
    {
        if ((input[i] >= 'a' && input[i] <= 'z') || (input[i] >= 'A' && input[i] <= 'Z'))
        {
            letter[j++] = input[i];
        }
        else if (input[i] >= '0' && input[i] <= '9')
        {
            digit[k++] = input[i];
        }

        i++;
    }
    letter[j] = '\0';
    digit[k] = '\0';
}

void handleInput(int client_sock)
{
    char buffer[BUFFER_SIZE];
    char sha256_hex_buffer[SHA256_HEX_SIZE];
    while (1)
    {
        memset(buffer, 0, BUFFER_SIZE);
        int bytes_read = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (bytes_read <= 0)
            break;

        if (strncmp(buffer, "FILE:", 5) == 0)
        {
            printf("%s\n", buffer + 5);
        }

        else
        {
            buffer[bytes_read] = '\0';
            buffer[strcspn(buffer, "\n")] = 0;
            if (!is_valid_string(buffer))
            {
                char error_message[] = "Invalid character\n";
                send(client_sock, error_message, strlen(error_message), 0);
                printf("Invalid character\n");
                continue;
            }
            sha256_hex(buffer, strlen(buffer), sha256_hex_buffer);
            char letter[SHA256_HEX_SIZE], digit[SHA256_HEX_SIZE];
            handle_string(sha256_hex_buffer, letter, digit);
            printf("%s\n", letter);
            printf("%s\n", digit);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Invalid command Line Argument!\n");
        return (0);
    }

    int PORT = atoi(argv[1]);       // server port
    int server_sock;                // server socket id
    int client_sock;                // client socket id
    struct sockaddr_in server_addr; // server's address information
    struct sockaddr_in client_addr; // client's address information
    socklen_t addr_len = sizeof(client_addr);

    // Construct a TCP socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        printf("create server socket fail\n");
        return 0;
    }

    // Bind address to socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("associate server socket with address fail\n");
        close(server_sock);
        return 0;
    }

    // listen connection
    if (listen(server_sock, 5) < 0)
    {
        printf("Listen connection failed!");
        close(server_sock);
        return (0);
    }

    printf("Server listening on port %d\n", PORT);

    // create a new socket to connect with client
    while ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len)) >= 0)
    {
        handleInput(client_sock);
    }

    close(server_sock);
    return 0;
}
