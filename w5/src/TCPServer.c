/*
 * tcpserver.c - A simple TCP echo server
 * usage: tcpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "user.h"
#include <ctype.h>

#define BUFSIZE 1024

#if 0
/* 
 * Structs exported from in.h
 */

/* Internet address */
struct in_addr {
  unsigned int s_addr; 
};

/* Internet style socket address */
struct sockaddr_in  {
  unsigned short int sin_family; /* Address family */
  unsigned short int sin_port;   /* Port number */
  struct in_addr sin_addr;	 /* IP address */
  unsigned char sin_zero[...];   /* Pad to size of 'struct sockaddr' */
};

/*
 * Struct exported from netdb.h
 */

/* Domain name service (DNS) host entry */
struct hostent {
  char    *h_name;        /* official name of host */
  char    **h_aliases;    /* alias list */
  int     h_addrtype;     /* host address type */
  int     h_length;       /* length of address */
  char    **h_addr_list;  /* list of addresses */
}
#endif

/*
 * error - wrapper for perror
 */
void error(char *msg)
{
    perror(msg);
    exit(1);
}
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
void parseRequest(char *buf, int *command, char data[][BUFSIZE])
{
    char *token = strtok(buf, ",");
    *command = atoi(token);
    switch (*command)
    {
    case 1:
        token = strtok(NULL, ",");
        strcpy(data[0], token);
        token = strtok(NULL, ",");
        strcpy(data[1], token);
        break;
    case 2:
        token = strtok(NULL, ",");
        strcpy(data[0], token);
        break;
    case 3:
        break; /*bye*/
    default:
        break;
    }
}
void login(Dllist list, User *user, char data[][BUFSIZE], char *mes)
{
    User find = existsUser(list, data[0]);
    if (!find)
    {
        strcpy(mes, "1,Cannot find account");
        return;
    }
    if (find->status == blocked)
    {
        strcpy(mes, "1,Account was blocked");
        return;
    }
    if (find->status == idle)
    {
        strcpy(mes, "1,Account is not active");
        return;
    }
    if (!checkPassword(find, data[1]))
    {
        strcpy(mes, "1,Not OK");
        find->errCount++;
        if (find->errCount >= 3)
        {
            blockUser(find);
            export(list, "users.txt");
            strcpy(mes, "1,Account is blocked");
        }
        else
            strcpy(mes, "1,Password incorrect");
        return;
    }
    strcpy(mes, "0,OK");
    *user = find;
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
void changePassword(Dllist list, User user, char *newPassword, char *mes)
{
    int checkPassword = 0;
    for (int i = 0; i < strlen(newPassword); i++)
    {
        if (!((newPassword[i] >= '0' && newPassword[i] <= '9') || newPassword[i] >= 'a' && newPassword[i] <= 'z' || newPassword[i] >= 'A' && newPassword[i] <= 'Z'))
        {
            checkPassword = 1;
            break;
        }
    }
    if (checkPassword)
    {
        strcpy(mes, "1,Password contains special characters");
        return;
    }
    updatePassword(user, newPassword);
    export(list, "users.txt");
    char *encodePass = encode(newPassword);
    sprintf(mes, "0,%s", encodePass);
}

void bye(char *mes)
{
    strcpy(mes, "0,bye");
}
int main(int argc, char **argv)
{
    int parentfd;                  /* parent socket */
    int childfd;                   /* child socket */
    int PORT;                      /* port to listen on */
    int clientlen;                 /* byte size of client's address */
    struct sockaddr_in serveraddr; /* server's addr */
    struct sockaddr_in clientaddr; /* client addr */
    struct hostent *hostp;         /* client host info */
    char buf[BUFSIZE];             /* message buffer */
    char *hostaddrp;               /* dotted decimal host addr string */
    int optval;                    /* flag value for setsockopt */
    int n;                         /* message byte size */

    const Dllist Databases = makUsersList("users.txt");
    User user = NULL;
    int command;
    char data[2][BUFSIZE];
    /*
     * check command line arguments
     */
    PORT = validdateParams(argc, argv);
    /*
     * socket: create the parent socket
     */
    parentfd = socket(AF_INET, SOCK_STREAM, 0);
    if (parentfd < 0)
        error("ERROR opening socket");

    /* setsockopt: Handy debugging trick that lets
     * us rerun the server immediately after we kill it;
     * otherwise we have to wait about 20 secs.
     * Eliminates "ERROR on binding: Address already in use" error.
     */
    optval = 1;
    setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR,
               (const void *)&optval, sizeof(int));

    /*
     * build the server's Internet address
     */
    bzero((char *)&serveraddr, sizeof(serveraddr));

    /* this is an Internet address */
    serveraddr.sin_family = AF_INET;

    /* let the system figure out our IP address */
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* this is the port we will listen on */
    serveraddr.sin_port = htons((unsigned short)PORT);

    /*
     * bind: associate the parent socket with a port
     */
    if (bind(parentfd, (struct sockaddr *)&serveraddr,
             sizeof(serveraddr)) < 0)
        error("ERROR on binding");

    /*
     * listen: make this socket ready to accept connection requests
     */
    if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */
        error("ERROR on listen");

    /*
     * main loop: wait for a connection request, echo input line,
     * then close connection.
     */
    clientlen = sizeof(clientaddr);
    childfd = accept(parentfd, (struct sockaddr *)&clientaddr, &clientlen);
    if (childfd < 0)
        error("ERROR on accept");
    hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                          sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
        error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
        error("ERROR on inet_ntoa\n");
    printf("server established connection with %s (%s)\n",
           hostp->h_name, hostaddrp);

    int isLoop = 1;
    while (isLoop)
    {
        /*
         * accept: wait for a connection request
         */

        /*
         * gethostbyaddr: determine who sent the message
         */

        /*
         * read: read input string from the client
         */
        bzero(buf, BUFSIZE);
        n = read(childfd, buf, BUFSIZE);
        if (n < 0)
            error("ERROR reading from socket");
        // printf("server received %d bytes: %s", n, buf);
        parseRequest(buf, &command, data);
        switch (command)
        {
        case 1:
            login(Databases, &user, data, buf);
            n = write(childfd, buf, strlen(buf));
            break;
        case 2:
            changePassword(Databases, user, data[0], buf);
            n = write(childfd, buf, strlen(buf));
            /* code */
            break;
        case 3:
            user = NULL;
            n = write(childfd, buf, strlen(buf));
            isLoop = 0;
            /* code */
            break;
        default:
            break;
        }
        if (n < 0)
            error("ERROR writing to socket");
    }
    close(childfd);
}
