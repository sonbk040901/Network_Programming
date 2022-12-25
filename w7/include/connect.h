#ifndef CONNECT_H_
#define CONNECT_H_
#include "user.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <errno.h>

typedef enum _command
{
    login,
    logout
} Command;
typedef struct _request
{
    Command command;
    struct _User user;
} Request;
typedef enum _resStatus
{
    success,
    fail
} ResStatus;
typedef struct _response
{
    ResStatus status;
    char message[255];
} Response;
int recvRequest(int sockfd, Request *request);
void initResponse(Response *response, ResStatus status, char message[]);
int sendResponse(int sockfd, Response *response);
void initRequest(Request *request, Command command, User user);
int sendRequest(int sockfd, Request *request);
int recvResponse(int sockfd, Response *response);

int recvRequest(int sockfd, Request *request)
{
    int bytesReceived = recv(sockfd, request, sizeof(Request), 0);
    if (bytesReceived < 0)
        perror("\nError recv req");
    if (!bytesReceived)
        printf("\nConnection close %s", strerror(errno));
    return bytesReceived;
}
int sendResponse(int sockfd, Response *response)
{
    int bytesSent = send(sockfd, response, sizeof(Response), 0);
    if (bytesSent < 0)
        perror("\nError send: ");
    return bytesSent;
}
void initResponse(Response *response, ResStatus status, char message[])
{
    bzero(response, sizeof(Response));
    response->status = status;
    strcpy(response->message, message);
}

void initRequest(Request *request, Command command, User user)
{
    request->command = command;
    if (user == NULL)
    {
        return;
    }
    request->user = *user;
}
int sendRequest(int sockfd, Request *request)
{
    int bytesSent = send(sockfd, request, sizeof(Request), 0);
    if (bytesSent < 0)
        perror("\nError send");
    return bytesSent;
}
int recvResponse(int sockfd, Response *response)
{
    int bytesReceived = recv(sockfd, response, sizeof(Response), 0);
    if (bytesReceived < 0)
        perror("\nError recv res");
    if (!bytesReceived)
        printf("\nConnection close %s", strerror(errno));
    return bytesReceived;
}
#endif