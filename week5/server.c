// server
#include <stdio.h>
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
    if (argc != 2)
    {
        printf("command line argument invalid!\n");
    }
    else
    {
        int PORT = atoi(argv[1]); // server port
        int server_sock;          // server socket id
        char buff[BUFF_SIZE];
        int bytes_sent, bytes_received;
        struct sockaddr_in server; // server's address information
        struct sockaddr_in client; // client1's address information
        int sin_size;

        // Step 1: Construct a UDP socket
        if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
        { // calls socket()
            printf("create server socket fail\n");
            return 0;
        }
        // Step 2: Bind address to socket
        bzero(&(server.sin_zero), 8); // zero the rest of the structure
        server.sin_family = AF_INET;
        server.sin_port = htons(PORT);       // Remember htons() from "Conversions" section? =)
        server.sin_addr.s_addr = INADDR_ANY; // INADDR_ANY puts your IP address automatically

        if (bind(server_sock, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
        { // calls bind()
            printf("associate server socket with address fail\n");
            return 0;
        }

        printf("server is running on port %d ...\n", PORT);

        // Step 3: Communicate with clients
        while (1)
        {
            sin_size = sizeof(struct sockaddr_in);

            // recieve from client
            bytes_received = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&client, &sin_size);

            if (bytes_received < 0)
                printf("error receive data from client\n");
            else
            {
                buff[bytes_received] = '\0';
            }

            // send to client2
            bytes_sent = sendto(server_sock, buff, bytes_received, 0, (struct sockaddr *)&client, sin_size); // send to the client2 reverse string
            if (bytes_sent < 0)
            {
                printf("error send data to client\n");
            }
        }

        close(server_sock);
        return 0;
    }
}