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
#define MAX_CHOSE 6
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
	printf("\t\t\t|======================================================|\n");
}
/*Must be free return value when done*/
char *prompt(char *message);
int main(void)
{
	int chose;
	Dllist list = makUsersList(USERS_DB);
	User user = NULL;
	char *username, *password, *activationCode;
	do
	{
		menu();
		printf("Your choice (1-6, other to quit): ");
		chose = scanf("%d", &chose) == 0 ? 0 : chose;
		__fpurge(stdin);
		switch (chose)
		{
		case 1:
			username = prompt("Username: ");
			if (existsUser(list, username))
			{
				puts("Account existed");
				free(username);
				break;
			}
			password = prompt("Password: ");
			addUser(list, newUser(username, password, idle));
			puts("Successful registration\nActivation required\n");
			export(list, USERS_DB);
			free(username);
			free(password);
			break;
		case 2:
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
				export(list, USERS_DB);
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
			export(list, USERS_DB);
			user = NULL;
			break;
		case 3:
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
			export(list, USERS_DB);
			break;
		case 4:
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
		case 5:
			if (!user)
			{
				puts("Account is not sign in");
				break;
			}
			printf("Username: %s\n", user->username);
			password = prompt("Password: ");
			if (!checkPassword(user, password))
			{
				puts("Current password is incorrect. Please try again");
				free(password);
				break;
			}
			free(password);
			password = prompt("NewPassword: ");
			setPassword(user, password);
			puts("Password is changed");
			free(password);
			export(list, USERS_DB);
			break;
		case 6:
			if (!user)
			{
				puts("Account is not sign in");
				break;
			}
			username = prompt("Username: ");
			result = existsUser(list, username);
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