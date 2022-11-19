// Client side implementation of UDP client-server model
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define MAXLINE 1024
int sockfd;
char dataBuffer[MAXLINE];
char hello[100];
struct sockaddr_in servaddr;
char username[100], password[100], message[100];
int bufLen, servaddrLen;
struct in_addr addr;
int validdateParams(int argc, char const **argv)
{
    if (argc != 3)
        goto errParam;
    int check = inet_aton(argv[1], &addr);
    if (!check)
        goto errParam;
    for (unsigned int i = 0; i < strlen(argv[2]); i++)
        if (!isdigit(argv[2][i]))
            goto errParam;
    int port = atoi(argv[2]);
    if (port < 1024 || port > 65535)
        goto errPort;
    return port;
errParam:
    puts("Wrong parameter\nRead README.md file to get help");
    exit(EXIT_FAILURE);
errPort:
    puts("Port number is from 1024(for safe: port<1024 is WKP) to 65535\nRead README.md file to get help");
    exit(EXIT_FAILURE);
}
void request(int type, char *buf1, char *buf2)
{
    char request[MAXLINE];
    if (type != 3)
    {
        sprintf(request, "%d,%s,%s", type, buf1, buf2);
    }
    else
        sprintf(request, "%d,%s", type, buf1);
    sendto(sockfd, request, sizeof(request), MSG_CONFIRM, (const struct sockaddr *)&servaddr, (socklen_t)servaddrLen);
}
int parseResponse(char *buf)
{
    char *token = strtok(buf, ",");
    int type = atoi(token);
    token = strtok(NULL, ",");
    strcpy(message, token);
    return type;
}
int main(int argc, const char *argv[])
{
    const int PORT = validdateParams(argc, argv);
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    bzero(&servaddr, sizeof(servaddr));
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr = addr;
    servaddrLen = sizeof(servaddr);
    int loop = 1;
    while (loop)
    {
        printf("Username: ");
        fgets(username, sizeof(username), stdin);
        printf("Password: ");
        fgets(password, sizeof(password), stdin);
        username[strlen(username) - 1] = '\0';
        password[strlen(password) - 1] = '\0';
        request(1, username, password);
        bufLen = recvfrom(sockfd, (void *)dataBuffer, MAXLINE,
                          MSG_WAITALL, (struct sockaddr *)&servaddr,
                          (socklen_t *)&servaddrLen);
        dataBuffer[bufLen] = '\0';
        int type = parseResponse(dataBuffer);
        if (type)
        {
            puts(message);
            continue;
        }
        loop = 0;
    }
    loop = 1;
    while (loop)
    {
        fgets(password, sizeof(password), stdin);
        password[strlen(password) - 1] = '\0';
        if (!strcmp(password, "bye"))
        {
            request(3, "bye", NULL);
            break;
        }
        request(2, password, NULL);
        bufLen = recvfrom(sockfd, (void *)dataBuffer, MAXLINE,
                          MSG_WAITALL, (struct sockaddr *)&servaddr,
                          (socklen_t *)&servaddrLen);
        dataBuffer[bufLen] = '\0';
        int type = parseResponse(dataBuffer);
        puts(message);
    }

    close(sockfd);
    return 0;
}
