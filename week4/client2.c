// client2
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define BUFF_SIZE 1024

int isValidString(const char *str)
{
    for (int i = 0; i < strlen(str); i++)
    {
        if (!isalpha(str[i]) && !isdigit(str[i]) && str[i] != '\0' && str[i] != '\n')
        {
            return -1;
        }
    }
    return 0;
}

void deleteDigits(char *str)
{
    int index = 0;
    for (int i = 0; i < strlen(str); i++)
    {
        if (isalpha(str[i]))
        {
            str[index++] = str[i];
        }
    }
    str[index] = '\0';
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("command line argument is invalid!\n");
    }
    else
    {
        char *SERV_IP = argv[1];       // server ip
        int SERV_PORT = atoi(argv[2]); // server port
        int client_sock;               // client2 socket id
        char buff[BUFF_SIZE];
        struct sockaddr_in client_addr;
        struct sockaddr_in server_addr;
        int bytes_sent, bytes_received, sin_size;

        // Step 1: Construct a UDP socket
        if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        { /* calls socket() */
            printf("error create socket\n");
            return 0;
        }

        printf("starting ...\n");

        // Step 2: Define the address of the server
        bzero(&server_addr, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(SERV_PORT);
        server_addr.sin_addr.s_addr = inet_addr(SERV_IP);

        // define the address of client2
        bzero(&client_addr, sizeof(client_addr));
        client_addr.sin_family = AF_INET;
        client_addr.sin_addr.s_addr = INADDR_ANY;
        client_addr.sin_port = htons(SERV_PORT + 1);

        // bind
        if (bind(client_sock, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0)
        {
            printf("Bind socket client2 failed\n");
            return 0;
        }

        // Step 3: Communicate with server
        while (1)
        {
            sin_size = sizeof(struct sockaddr);

            // receive data
            bytes_received = recvfrom(client_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&server_addr, &sin_size); // receive reversed string from server
            if (bytes_received < 0)
            {
                printf("Error receive data from server\n");
                return 0;
            }
            buff[bytes_received] = '\0';
            if (isValidString(buff) == -1)
            {
                printf("string recieved from server has invalid character\n");
            }
            else
            {
                deleteDigits(buff);
                printf("%s\n", buff);
            }
        }
        close(client_sock);
        return 0;
    }
}
