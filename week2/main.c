#include <stdio.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void domainNameToIp(char *domainName)
{
    struct hostent *ghbn = gethostbyname(domainName);
    if (ghbn)
    {
        char **prtToList = ghbn->h_addr_list; // pointer to a list of char* ip address

        // official ip
        struct in_addr *ipv4addr = (struct in_addr *)*prtToList; // convert char* ip address to ipv4 address
        printf("Official IP: %s\n", inet_ntoa(*ipv4addr));       // ipv4 string

        // alias ip
        printf("Alias IP:\n");
        for (char **p = prtToList + 1; *p != NULL; p++)
        {
            struct in_addr *ipv4addr = (struct in_addr *)*p;
            printf("%s\n", inet_ntoa(*ipv4addr));
        }
    }
    else
    {
        printf("Not found information\n");
    }
}

void ipToDomainName(char *ipAddress)
{
    struct in_addr ipv4addr;

    if (inet_pton(AF_INET, ipAddress, &ipv4addr) == 1)
    {
        struct hostent *ghbi = gethostbyaddr(&ipv4addr, sizeof(ipv4addr), AF_INET);
        if (ghbi)
        {
            printf("Offical name: %s\n", ghbi->h_name); // offical name

            char **aliases = ghbi->h_aliases; // point to list of alias name
            printf("Alias name:\n");
            for (char **p = aliases; *p != NULL; p++)
            {
                printf("%s\n", *p);
            }
        }
        else
        {
            printf("Not found information\n");
        }
    }
    else
    {
        printf("Not found information\n");
    }
}
int checkInput(char *input)
{
    int dotCount = 0;
    int numberCount = 0;
    int characterCount = 0;
    for (char *p = input; *p != '\0'; p++)
    {
        if (*p == '.')
        {
            dotCount++;
        }
        else if (*p >= '0' && *p <= '9')
        {
            numberCount++;
        }
        else if ((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z'))
        {
            characterCount++;
        }
    }
    if (dotCount == 2 || (characterCount == 0 && dotCount == 0))
    {
        return -1;
    }
    return 0;
}

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        printf("Invalid command Line Argument!\n");
    }
    else
    {
        char *input = argv[1];
        struct in_addr ipv4addr;
        if (inet_pton(AF_INET, input, &ipv4addr) == 1)
        {
            ipToDomainName(input);
        }
        else
        {
            if (checkInput(input) > -1)
            {
                domainNameToIp(input);
            }
            else
            {
                printf("Not found information\n");
            }
        }
    }
    return 0;
}
