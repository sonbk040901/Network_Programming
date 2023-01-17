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
#include <sys/time.h>
#define BUFSIZE 1024
#define MAX_CLIENT 30
#define TIMEOUT 5
int validdateParams(int argc, char const *argv[]);
void error(char *msg);
int handle_client(int fd);
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
    int clientfds[MAX_CLIENT];
    int max_fd = 0;
    int activity = 0;
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
    clientlen = sizeof(struct sockaddr_in);

    fd_set readfds;
    for (int i = 0; i < MAX_CLIENT; i++)
    {
        clientfds[i] = -1;
    }
    max_fd = 0;
    int i = 0;
    while (true)
    {
        FD_ZERO(&readfds);
        FD_SET(parentfd, &readfds);
        printf("waiting for connection...\n");
        max_fd = parentfd;
        for (int i = 0; i < MAX_CLIENT; i++)
        {
            childfd = clientfds[i];
            if (childfd > max_fd)
            {
                max_fd = childfd;
            }
            if (childfd != -1)
            {
                FD_SET(childfd, &readfds);
            }
        }
        struct timeval timeout;
        timeout.tv_sec = TIMEOUT;
        activity = select(max_fd + 2, &readfds, NULL, NULL, NULL);
        if (FD_ISSET(parentfd, &readfds))
        {
            if ((childfd = accept(parentfd, (struct sockaddr *)&clientaddr, (socklen_t *)&clientlen)) == -1)
            {
                if (errno == EINTR)
                    continue;
                else
                    perror("\nError accepting: ");
            }
            printf("\nConnected to (%s, %d)\n", inet_ntoa(clientaddr.sin_addr), ntohs(clientaddr.sin_port));
            FD_SET(childfd, &readfds);
            for (int i = 0; i < MAX_CLIENT; i++)
            {
                if (clientfds[i] == -1)
                {
                    FD_SET(childfd, &readfds);
                    max_fd = childfd > max_fd ? childfd : max_fd;
                    clientfds[i] = childfd;
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CLIENT; i++)
        {
            childfd = clientfds[i];
            if (childfd == -1)
            {
                continue;
            }
            if (FD_ISSET(childfd, &readfds))
            {

                n = handle_client(childfd);
                if (n)
                {
                    clientfds[i] = n;
                }
            }
        }
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
    puts("test");
    return port;
errParam:
    fprintf(stderr, "Wrong parameter\nRead README.md file to get help\n");
    exit(EXIT_FAILURE);
errPort:
    fprintf(stderr, "Port number is from 1024(for safe: port<1024 is WKP) to 65535\nRead README.md file to get help\n");
    exit(EXIT_FAILURE);
}
int handle_client(int fd)
{
    Request req;
    Response res;
    Dllist userList = NULL;
    User user = NULL;
    int recvBytes = recvRequest(fd, &req);
    if (recvBytes <= 0)
    {
        close(fd);
        return -1;
    }

    switch (req.command)
    {
    case login:
        userList = makeUsersList("users.txt");
        user = existsUser(userList, req.user.username);
        if (!user)
        {
            initResponse(&res, fail, "User is not exists");
            goto doneCase1;
        }
        asyncErrCount(user);
        if (user->errCount >= 3)
        {
            initResponse(&res, fail, "User was locked");
            goto doneCase1;
        }
        if (!checkPassword(user, req.user.password))
        {
            user->errCount++;
            printf("\nerrCount: %d\n", user->errCount);
            if (user->errCount >= 3)
            {
                initResponse(&res, fail, "User is locked");
                blockUser(user);
            }
            else
                initResponse(&res, fail, "Wrong password");
            goto doneCase1;
        }
        initResponse(&res, success, "Login successfully");
        user->errCount = 0;
    doneCase1:
        sendResponse(fd, &res);
        exportList(userList, "users.txt");
        freeUserList(userList);
        break;
    case change_password:
        userList = makeUsersList("users.txt");
        if ((user = existsUser(userList, req.user.username)) == NULL)
        {
            initResponse(&res, fail, "User is not exists");
            printf("err: %s\n", req.user.username);
            break;
        }
        char *newPassword = req.user.password;
        if (strcmp(newPassword, "bye") == 0)
        {

            initResponse(&res, fail, "Logout");
            sendResponse(fd, &res);
            exportList(userList, "users.txt");
            freeUserList(userList);
            break;
        }

        strcpy(user->password, newPassword);
        initResponse(&res, success, "Change password successfully");
        sendResponse(fd, &res);
        exportList(userList, "users.txt");
        freeUserList(userList);
        break;
    default:
        break;
    }
    return 0;
}
void error(char *msg)
{
    perror(msg);
    exit(1);
}