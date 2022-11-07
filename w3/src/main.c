#include <stdio.h>
#include <stdlib.h>
#include <stdio_ext.h>
#include <ctype.h>
#include <string.h>
#include "dllist.h"
#include "fields.h"
#include "jrb.h"
#include "jval.h"
#include "queue.h"
#include "user.h"
#include <sys/socket.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <ctype.h>
#define MAX_CHOSE 8
#define USERS_DB "users.txt"
void menu(void)
{
	printf("\t\t\t|======================================================|\n");
	printf("\t\t\t|%15sUSER MANAGEMENT PROGRAM%16s|\n", " ", " ");
	printf("\t\t\t|------------------------------------------------------|\n");
	printf("\t\t\t|  1  |   %-45s|\n", "Register");
	printf("\t\t\t|  2  |   %-45s|\n", "Activate");
	printf("\t\t\t|  3  |   %-45s|\n", "Sign in");
	printf("\t\t\t|  4  |   %-45s|\n", "Search");
	printf("\t\t\t|  5  |   %-45s|\n", "Change password");
	printf("\t\t\t|  6  |   %-45s|\n", "Sign out");
	printf("\t\t\t|  7  |   %-45s|\n", "Homepage with domain name");
	printf("\t\t\t|  8  |   %-45s|\n", "Homepage with IP address");
	printf("\t\t\t|======================================================|\n");
}
/*Must be free return value when done*/
char *prompt(char *message);
struct hostent *gethost(char *, int);
int main(void)
{
	int chose;
	Dllist list = makUsersList(USERS_DB);
	User user = NULL;
	char *username, *password, *activationCode, *homepage;
	do
	{
		menu();
		printf("Your choice (1-8, other to quit): ");
		chose = scanf("%d", &chose) == 0 ? 0 : chose;
		__fpurge(stdin);
		switch (chose)
		{
		case 1:
		{
			username = prompt("Username: ");
			if (existsUser(list, username))
			{
				puts("Account existed");
				free(username);
				break;
			}
			password = prompt("Password: ");
			homepage = prompt("Homepage: ");
			addUser(list, newUser(username, password, homepage, idle));
			puts("Successful registration\nActivation required\n");
			exportFile(list, USERS_DB);
			free(username);
			free(password);
			break;
		}
		case 2:
		{
			int count = 0;
			do
			{
				username = prompt("Username: ");
				user = existsUser(list, username);
				if (!user)
				{
					puts("Cannot find account");
					free(username);
					continue;
				}
				if (user->status == active)
				{
					puts("Account was activated");
					free(username);
					continue;
				}

				if (user->status == blocked)
				{
					puts("Account was blocked");
					free(username);
					continue;
				}
				count = 0;
				do
				{
					password = prompt("Password: ");
					if (!checkPassword(user, password))
					{
						puts("Password is incorrect");
						++count;
					}
					else
					{
						// puts("Login successful");
						break;
					}
					if (count >= 3)
					{
						blockUser(user);
						puts("Account is blocked");
						break;
					}
					free(password);
				} while (1);
				break;
			} while (1);
			free(username);
			free(password);
			if (count >= 3)
			{
				exportFile(list, USERS_DB);
				break;
			}

			int activateCount = 0;
			do
			{
				activationCode = prompt("Activation code: ");
				if (!activate(user, activationCode))
				{
					puts("Account is not activated");
					++activateCount;
				}
				else
				{
					puts("Account is activated");
					break;
				}
				if (activateCount >= 4)
				{
					blockUser(user);
					puts("Account is blocked");
					break;
				}
				free(activationCode);
			} while (1);
			exportFile(list, USERS_DB);
			user = NULL;
			break;
		}
		case 3:
		{
			if (user)
			{
				printf("You must logout current user!!!\n");
				break;
			}
			do
			{
				username = prompt("Username: ");
				user = existsUser(list, username);
				if (!user)
				{
					puts("Cannot find account");
					free(username);
					continue;
				}
				if (user->status == idle)
				{
					puts("Account is idle");
					user = NULL;
					free(username);
					continue;
				}

				if (user->status == blocked)
				{
					puts("Account was blocked");
					user = NULL;
					free(username);
					continue;
				}
				int count = 0;
				do
				{
					password = prompt("Password: ");
					if (!checkPassword(user, password))
					{
						puts("Password is incorrect");
						++count;
					}
					else
					{
						printf("Hello %s\n", getUserName(user));
						break;
					}
					if (count >= 3)
					{
						blockUser(user);
						puts("Account is blocked");
						user = NULL;
						break;
					}
					free(password);
				} while (1);
				break;
			} while (1);
			free(username);
			free(password);
			exportFile(list, USERS_DB);
			break;
		}
		case 4:
		{
			if (!user)
			{
				puts("Account is not sign in");
				break;
			}
			username = prompt("Username: ");
			User result = existsUser(list, username);
			if (!result)
			{
				puts("Cannot find account");
				free(username);
				break;
			}
			else
			{
				Status status = getUserStatus(result);
				printf("Account is %s\n", status == blocked ? "blocked" : status == active ? "active"
																						   : "idle");
			}
			break;
		}
		case 5:
		{
			if (!user)
			{
				puts("Account is not sign in");
				break;
			}
			// printf("Username: %s\n", user->username);
			username = prompt("Username: ");
			password = prompt("Password: ");
			if (strcmp(user->username, username) || !checkPassword(user, password))
			{
				puts("Current username or password is incorrect. Please try again");
				free(password);
				break;
			}
			free(password);
			password = prompt("NewPassword: ");
			setPassword(user, password);
			puts("Password is changed");
			free(password);
			exportFile(list, USERS_DB);
			break;
		}
		case 6:
		{
			if (!user)
			{
				puts("Account is not sign in");
				break;
			}
			username = prompt("Username: ");
			User result = existsUser(list, username);
			if (!result)
			{
				puts("Cannot find account");
				free(username);
				break;
			}
			else
			{
				printf("Goodbye %s\n", getUserName(user));
			}
			user = NULL;
			break;
		}
		case 7:
		{
			if (!user)
			{
				puts("Account is not sign in");
				break;
			}
			struct hostent *he;
			he = gethost(user->homepage, 1);
			if (he == (void *)1)
			{
				puts("Wrong IP format");
				break;
			}
			if (he == NULL)
			{
				puts("Not found information");
				break;
			}
			printf("Info of IP %s:\n", user->homepage);
			printf("Official name: %s\n", he->h_name);
			printf("Alias name:\n");
			for (int i = 0; he->h_aliases[i] != NULL; i++)
			{
				printf(" %s\n", he->h_aliases[i]);
			}
			break;
		}
		case 8:
		{
			if (!user)
			{
				puts("Account is not sign in");
				break;
			}
			struct hostent *he;
			struct in_addr **addr_list;
			he = gethost(user->homepage, 2);
			if (he == (void *)1)
			{
				puts("Wrong domain name format");
				break;
			}
			if (he == NULL)
			{
				puts("Not found information");
				break;
			}
			printf("Info of domain name %s:\n", user->homepage);
			printf("Official IP: %s\n", inet_ntoa(*(struct in_addr *)he->h_addr));
			printf("Alias IP:\n");
			addr_list = (struct in_addr **)he->h_addr_list;
			for (int i = 1; addr_list[i] != NULL; i++)
			{
				printf("%s\n", inet_ntoa(*addr_list[i]));
			}
			break;
		}
		default:
			freeUserList(list);
			free_dllist(list);
			printf("\tEnd program\n");
			chose = 0;
			break;
		}
	} while (chose);
	return 0;
}
char *prompt(char *message)
{
	char result[100], *ptr = NULL;
	do
	{
		printf("%s", message);
		fgets(result, 100, stdin);
		result[strlen(result) - 1] = 0;
		ptr = strstr(result, " ");
		if (ptr)
		{
			puts("Input not available(have space)");
			continue;
		}
	} while (strlen(result) == 0 || ptr);
	return strdup(result);
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