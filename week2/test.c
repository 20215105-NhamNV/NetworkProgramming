#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define API_KEY "17400875130ee06a1bb9f927e5106a9729b8dc3c52954fe46fefd27c5a28fb88" // Thay thế bằng khóa API của bạn

void checkMaliciousDomain(char *domain)
{
    int sock;
    struct sockaddr_in server;
    char request[512], response[4096];

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("Tạo socket thất bại");
        return;
    }

    server.sin_family = AF_INET;
    server.sin_port = htons(80);
    server.sin_addr.s_addr = inet_addr("104.16.25.35"); // Địa chỉ IP của máy chủ API VirusTotal

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Kết nối thất bại");
        close(sock);
        return;
    }

    sprintf(request, "GET /vtapi/v2/url/report?apikey=%s&resource=%s HTTP/1.1\r\nHost: www.virustotal.com\r\nConnection: close\r\n\r\n", API_KEY, domain);
    send(sock, request, strlen(request), 0);

    int bytes_received = recv(sock, response, sizeof(response) - 1, 0);
    if (bytes_received > 0)
    {
        response[bytes_received] = '\0';
        printf("%s\n", response);
    }

    close(sock);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Sử dụng: %s <tên miền>\n", argv[0]);
        return 1;
    }

    char *domain = argv[1];
    checkMaliciousDomain(domain);
    return 0;
}
