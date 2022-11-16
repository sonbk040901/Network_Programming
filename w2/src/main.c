#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#include <string.h>
struct hostent *gethost(char *str, int mode);

int main(int argc, char *argv[])
{
	if (argc != 3 || strlen(argv[1]) != 1 || !isdigit(argv[1][0]) || (atoi(argv[1]) != 1 && atoi(argv[1]) != 2))
	{
		puts("Wrong parameter");
		exit(1);
	}
	struct hostent *he;
	struct in_addr **addr_list;
	int mode = atoi(argv[1]);
	he = gethost(argv[2], mode);
	if (he == (void *)1)
	{
		puts("Wrong parameter");
		exit(1);
	}
	if (he == NULL)
	{
		puts("Not found information");
		exit(1);
	}
	if (mode == 2)
	{
		printf("Official IP: %s\n", inet_ntoa(*(struct in_addr *)he->h_addr));
		printf("Alias IP:\n");
		addr_list = (struct in_addr **)he->h_addr_list;
		for (int i = 1; addr_list[i] != NULL; i++)
		{
			printf("%s\n", inet_ntoa(*addr_list[i]));
		}
	}
	else
	{
		printf("Official name: %s\n", he->h_name);
		printf("Alias name:\n");
		for (int i = 0; he->h_aliases[i] != NULL; i++)
		{
			printf(" %s\n", he->h_aliases[i]);
		}
	}
	return 0;
}
struct hostent *gethost(char *str, int mode)
{
	struct in_addr addr;
	int check = inet_aton(str, &addr);
	if ((check && mode == 2) || (!check && mode == 1))
	{
		return (void *)1;
	}
	if (mode == 2)
	{
		return gethostbyname(str);
	}
	else
	{
		return gethostbyaddr(&addr, sizeof(addr), AF_INET);
	}
}