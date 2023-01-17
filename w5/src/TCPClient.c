/*
 * tcpclient.c - A simple TCP client
 * usage: tcpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <ctype.h>
#define BUFSIZE 1024

/*
 * error - wrapper for perror
 */
void error(char *msg)
{
    perror(msg);
    exit(0);
}
int validdateParams(int argc, char const **argv)
{
    if (argc != 3)
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
void login(char *mes)
{
    char username[512], password[512];
    printf("User name: ");
    fgets(username, sizeof(username), stdin);
    username[strlen(username) - 1] = '\0';
    printf("Password: ");
    fgets(password, sizeof(password), stdin);
    password[strlen(password) - 1] = '\0';
    sprintf(mes, "1,%s,%s", username, password);
}
void changePassword(char *mes)
{
    char newPassword[512];
    printf("New password: ");
    fgets(newPassword, sizeof(newPassword), stdin);
    newPassword[strlen(newPassword) - 1] = '\0';
    sprintf(mes, "%d,%s", strcmp(newPassword, "bye") == 0 ? 3 : 2, newPassword);
}
void parseResponse(char *buf, int *isLogin, int *isLoop)
{
    char *token = strtok(buf, ",");
    int status = atoi(token);
    token = strtok(NULL, ",");
    if (*isLogin)
    {
        *isLoop = strcmp(token, "bye");
    }
    else
    {
        *isLogin = !status;
    }
    strcpy(buf, token);
}
int main(int argc, char **argv)
{
    int sockfd, portno, n;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];

    /* check command line arguments */
    portno = validdateParams(argc, argv);
    hostname = argv[1];

    int isLogin = 0, isLoop = 1;
    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    /* gethostbyname: get the server's DNS entry */
    server = gethostbyname(hostname);
    if (server == NULL)
    {
        fprintf(stderr, "ERROR, no such host as %s\n", hostname);
        exit(0);
    }

    /* build the server's Internet address */
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);
    int i = 0;
    if (connect(sockfd, (struct sockaddr *)&serveraddr, (socklen_t)sizeof(serveraddr)) < 0)
        error("ERROR connecting");
    while (isLoop)
    {
        if (!isLogin)
        {
            login(buf);
        }
        else
        {
            changePassword(buf); //
        }
        /* send the message line to the server */
        n = write(sockfd, buf, strlen(buf));
        if (n < 0)
            error("ERROR writing to socket");
        /* print the server's reply */
        bzero(buf, BUFSIZE);
        n = read(sockfd, buf, BUFSIZE);
        parseResponse(buf, &isLogin, &isLoop);
        puts(buf);
        if (n < 0)
            error("ERROR reading from socket");
    }
    close(sockfd);
    return 0;
}
