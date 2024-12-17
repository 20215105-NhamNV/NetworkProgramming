/*UDP Echo Client*/
#include <stdio.h> /* These are the usual header files */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFF_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("command line argument is invalid!\n");
    }
    else
    {
        char *SERV_IP = argv[1]; // server port
        int SERV_PORT = atoi(argv[2]);
        int client_sock;
        char sendedBuff[BUFF_SIZE];
        char receivedBuff[BUFF_SIZE];
        struct sockaddr_in server_addr;
        int bytes_sent, bytes_received, sin_size;

        // Step 1: Construct a UDP socket
        if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        { /* calls socket() */
            printf("error create socket\n");
            return 0;
        }

        // Step 2: Define the address of the server
        bzero(&server_addr, sizeof(server_addr)); // init properties of server_addr = 0
        server_addr.sin_family = AF_INET;         // ipv4
        server_addr.sin_port = htons(SERV_PORT);  // host byte order to network byte order,
        server_addr.sin_addr.s_addr = inet_addr(SERV_IP);

        // Step 3: Communicate with server

        while (1)
        {
            char username[30];
            char password[30];
            printf("Enter username and password (separated by a space):\n--(Enter empty string to exit)--\n");
            scanf("%s %s", username, password);
            if (strchr(username, '\0') != NULL)
            {
                printf("Exit\n");
                break;
            }
            sprintf(sendedBuff, "%s %s", username, password);

            sin_size = sizeof(struct sockaddr);

            bytes_sent = sendto(client_sock, sendedBuff, strlen(sendedBuff), 0, (struct sockaddr *)&server_addr, sin_size);
            if (bytes_sent < 0)
            {
                printf("error send data to server\n");
                close(client_sock);
            }

            // receive data
            bytes_received = recvfrom(client_sock, receivedBuff, BUFF_SIZE - 1, 0, (struct sockaddr *)&server_addr, &sin_size); //
            printf("%s\n", receivedBuff);
        }

        close(client_sock);
        return 0;
    }
}
