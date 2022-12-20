#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <user.h>
#include <stdbool.h>
#include "connect.h"
#define BUFSIZE 1024
int validdateParams(int argc, char const *argv[]);
void connectHandler(int sockfd);
void signalHandler(int signo);
void error(char *msg);
int main(int argc, char const *argv[])
{
    int PORT = validdateParams(argc, argv); /* port to listen on */
    int parentfd;                           /* parent socket */
    int childfd;                            /* child socket */
    int clientlen;                          /* byte size of client's address */
    struct sockaddr_in serveraddr;          /* server's addr */
    struct sockaddr_in clientaddr;          /* client addr */
    struct hostent *hostp;                  /* client host info */
    char *hostaddrp;                        /* dotted decimal host addr string */
    int optval;                             /* flag value for setsockopt */
    int n;                                  /* message byte size */

    pid_t pid;
    parentfd = socket(AF_INET, SOCK_STREAM, 0);
    if (parentfd < 0)
    {
        error("\nSocket error: ");
    }
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(PORT);
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(parentfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) == -1)
    {
        error("\nError: ");
    }
    if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */
        error("\nError on listen: ");
    signal(SIGCHLD, signalHandler);
    while (true)
    {
        clientlen = sizeof(struct sockaddr_in);
        if ((childfd = accept(parentfd, (struct sockaddr *)&clientaddr, &clientlen)) == -1)
        {

            if (errno == EINTR)
                continue;
            else
                perror("\nError accepting: ");
        }
        pid = fork();
        if (pid == 0) // is the child process
        {
            // TODO: echo client profile
            close(parentfd);
            printf("Connection from %s:%d\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
            connectHandler(childfd);
            exit(EXIT_SUCCESS); // exit the child process when handler done
        }
        // is the parent process
        close(childfd);
    }
    close(parentfd);
    return 0;
}
/// @brief
/// @param argc
/// @param argv
/// @return port
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
    fprintf(stderr, "Wrong parameter\nRead README.md file to get help\n");
    exit(EXIT_FAILURE);
errPort:
    fprintf(stderr, "Port number is from 1024(for safe: port<1024 is WKP) to 65535\nRead README.md file to get help\n");
    exit(EXIT_FAILURE);
}
void connectHandler(int sockfd)
{
    Dllist userList = NULL;
    User user = NULL;
    Request request;
    Response response;
    // int bytesSent // the number of bytes to send
    //     ,
    //     bytesReceived; // the number of bytes to receive
    int isLogout = 0;
    while (true)
    {
        recvRequest(sockfd, &request);
        switch (request.command)
        {
        case login:
            userList = makeUsersList("users.txt");
            user = existsUser(userList, request.user.username);
            if (!user)
            {
                initResponse(&response, fail, "User is not exists");
                goto doneCase1;
            }
            asyncErrCount(user);
            if (user->errCount >= 3)
            {
                initResponse(&response, fail, "User was locked");
                goto doneCase1;
            }
            if (!checkPassword(user, request.user.password))
            {
                user->errCount++;
                printf("\nerrCount: %d\n", user->errCount);
                if (user->errCount >= 3)
                {
                    initResponse(&response, fail, "User is locked");
                    blockUser(user);
                }
                else
                    initResponse(&response, fail, "Wrong password");
                goto doneCase1;
            }
            initResponse(&response, success, "Login successfully");
            user->errCount = 0;
        doneCase1:
            sendResponse(sockfd, &response);
            exportList(userList, "users.txt");
            freeUserList(userList);
            break;
        case logout:
            initResponse(&response, success, "Logout successfully");
            sendResponse(sockfd, &response);
            isLogout = 1;
            /* code */
            break;
        default:
            break;
        }
        if (isLogout)
            break;
    }
    close(sockfd);
}
void signalHandler(int signo)
{
    pid_t pid;
    int stat;
    /* Wait the child process terminate */
    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        printf("\nChild %d terminated\n", pid);
}
void error(char *msg)
{
    perror(msg);
    exit(1);
}