#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string.h>
#include "connect.h"
#include "user.h"
#define BUFF_SIZE 1024
int validdateParams(int argc, char const *argv[]);

int main(int argc, char const *argv[])
{
    int client_sock;
    char buff[BUFF_SIZE + 1];
    char buff1[BUFF_SIZE + 1];
    struct sockaddr_in server_addr; /* server's address information */
    int msg_len, bytes_sent, bytes_received;
    Request request;
    Response response;
    const int SERVER_PORT = validdateParams(argc, argv); /* port to listen on */
    const char *SERVER_ADDR = argv[1];                   /* server's IP address (dotted quad) */
    // Step 1: Construct socket
    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    // Step 2: Specify server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    // Step 3: Request to connect server
    if (connect(client_sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) < 0)
    {
        printf("\nError!Can not connect to sever! Client exit imediately!\n");
        return 0;
    }

    // Step 4: Communicate with server
    int isLogin = 0;
    struct _User user;
    do
    {
        if (!isLogin)
        {
            printf("Login: \n");
            printf("Username: ");
            fgets(user.username, 100, stdin);
            user.username[strlen(user.username) - 1] = '\0';
            printf("Password: ");
            fgets(user.password, 100, stdin);
            user.password[strlen(user.password) - 1] = '\0';
            initRequest(&request, login, &user);
            sendRequest(client_sock, &request);
            recvResponse(client_sock, &response);
            isLogin = response.status == success;
            puts(response.message);
        }
        else
        {
            printf("Enter new password: ");
            fgets(user.password, 100, stdin);
            user.password[strlen(user.password) - 1] = '\0';
            if (strlen(user.password) == 0)
            {
                puts("bye");
                break;
            }

            initRequest(&request, change_password, &user);
            sendRequest(client_sock, &request);
            recvResponse(client_sock, &response);
            puts(response.message);
            if (strcmp(response.message, "bye") == 0)
            {
                isLogin = 0;
            }
        }
    } while (1);

    // Step 4: Close socket
    close(client_sock);
    return 0;
}
int validdateParams(int argc, char const *argv[])
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
    fprintf(stderr, "Wrong parameter\nRead README.md file to get help\n");
    exit(EXIT_FAILURE);
errPort:
    fprintf(stderr, "Port number is from 1024(for safe: port<1024 is WKP) to 65535\nRead README.md file to get help\n");
    exit(EXIT_FAILURE);
}