#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <user.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#define MAXLINE 1024
int sockfd;
struct sockaddr_in servaddr, cliaddr;
User user = NULL;
char username[100], password[100], requsetType[100], body[200];
char res[MAXLINE];
int cliaddrLength = sizeof(cliaddr), bufLen;
/// @brief
/// @param argc
/// @param argv
/// @return port number
int validdateParams(int argc, char const *argv[])
{
    if (argc != 2)
        goto errParam;
    for (unsigned int i = 0; i < strlen(argv[1]); i++)
        if (!isdigit(argv[1][i]))
            goto errParam;
    int port = atoi(argv[1]);
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
void response(int status, char *message)
{
    char buf[MAXLINE];
    sprintf(buf, "%d,%s", status, message);
    sendto(sockfd, buf, sizeof(buf), MSG_CONFIRM, (const struct sockaddr *)&cliaddr,
           (socklen_t)cliaddrLength);
}
int parseRequest(char *dataBuffer)
{
    char *token = strtok(dataBuffer, ",");
    char strs[2][100];
    int type = atoi(token);
    int num = type == 1 ? 2 : 1;
    for (int i = 0; i < num && token != NULL; i++)
    {
        token = strtok(NULL, ",");
        strcpy(strs[i], token);
    }
    if (type == 1)
    {
        strcpy(username, strs[0]);
        strcpy(password, strs[1]);
    }
    else
    {
        strcpy(password, strs[0]);
    }
    return type;
}
char *encode(char *pass)
{
    char ascii[256];
    bzero(ascii, sizeof(ascii));
    char encode[100];
    int encodeLen = 0;
    for (unsigned int i = 0; i < strlen(pass); i++)
    {
        ascii[(int)pass[i]]++;
    }
    for (int i = 0; i < 256; i++)
    {
        if (i == ('9' + 1))
        {
            encode[encodeLen++] = '\n';
        }
        if (ascii[i] > 0)
        {
            encode[encodeLen++] = (char)i;
        }
    }
    encode[encodeLen] = '\0';
    return strdup(encode);
}
int main(int argc, char const *argv[])
{
    const int PORT = validdateParams(argc, argv);
    const Dllist Databases = makUsersList("users.txt");
    char dataBuffer[MAXLINE];
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
    bzero(&servaddr, sizeof(servaddr));
    bzero(&cliaddr, sizeof(cliaddr));
    // Filling server information
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    servaddr.sin_port = htons(PORT);
    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening

    char loop = 1;
    //
    while (loop)
    {
        bufLen = recvfrom(sockfd, (void *)dataBuffer, MAXLINE,
                          MSG_WAITALL, (struct sockaddr *)&cliaddr,
                          (socklen_t *)&cliaddrLength);
        dataBuffer[bufLen] = '\0';
        int type = parseRequest(dataBuffer);
        switch (type)
        {
        case 1:
            user = existsUser(Databases, username);
            if (!user)
            {
                response(-1, "Cannot find account");
                continue;
            }
            if (user->status == blocked)
            {
                user = NULL;
                response(4, "Account was blocked");
                continue;
            }
            if (!checkPassword(user, password))
            {
                user->errCount++;
                if (user->errCount >= 3)
                {
                    blockUser(user);
                    export(Databases, "users.txt");
                    response(3, "Account is blocked");
                }
                else
                    response(2, "password incorrect");
                continue;
            }
            printf("Login: %s %s\n", username, password);
            response(0, "login success");
            continue;
            break;
        case 2:
            if (user == NULL)
            {
                response(1, "Must login first");
                continue;
            }
            int checkPassword = 0;
            for (int i = 0; i < strlen(password); i++)
            {
                if (!(password[i] >= '0' && password[i] <= '9' || password[i] >= 'a' && password[i] <= 'z' || password[i] >= 'A' && password[i] <= 'Z'))
                {
                    checkPassword = 1;
                    break;
                }
            }
            if (checkPassword)
            {
                response(2, "Password contains special characters");
                continue;
            }
            updatePassword(user, password);
            export(Databases, "users.txt");
            response(0, encode(password));
            printf("Change password: %s\n", password);
            continue;
            break;
        default:
            if (user == NULL)
            {
                response(1, "Must login first");
                continue;
            }
            printf("bye\n");
            freeUserList(Databases);
            free_dllist(Databases);
            loop = 0;
            break;
        }
    }
    close(sockfd);
    return 0;
}
