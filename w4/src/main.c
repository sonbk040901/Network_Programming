#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
int main(int argc, char const *argv[])
{
	printf("Hello World!\n");
	perror("errr");
	exit(1);
	return (0);
}