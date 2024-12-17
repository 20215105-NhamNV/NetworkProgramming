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

void reverse(char *str)
{
    int first = 0;
    int last = strlen(str) - 1;
    char temp;

    while (first < last)
    {
        temp = str[first];
        str[first] = str[last];
        str[last] = temp;

        first++;
        last--;
    }
}

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
        struct sockaddr_in server;  // server's address information
        struct sockaddr_in client1; // client1's address information
        struct sockaddr_in client2; // client2's address information
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

        // define address of client2
        bzero(&(client2.sin_zero), 8);
        client2.sin_family = AF_INET;
        client2.sin_port = htons(PORT + 1);
        client2.sin_addr.s_addr = inet_addr("127.0.0.1");

        printf("server is running on port %d ...\n", PORT);
        // Step 3: Communicate with clients
        while (1)
        {
            sin_size = sizeof(struct sockaddr_in);

            // recieve from client1
            bytes_received = recvfrom(server_sock, buff, BUFF_SIZE - 1, 0, (struct sockaddr *)&client1, &sin_size);

            if (bytes_received < 0)
                printf("error receive data from client\n");
            else
            {
                buff[bytes_received] = '\0';
            }

            reverse(buff); // reverse

            // send to client2
            bytes_sent = sendto(server_sock, buff, bytes_received, 0, (struct sockaddr *)&client2, sin_size); // send to the client2 reverse string
            if (bytes_sent < 0)
            {
                printf("error send data to client\n");
            }
        }

        close(server_sock);
        return 0;
    }
}