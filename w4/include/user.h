#ifndef _USER_H
#define _USER_H
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
#define ACTIVATION_CODE "20194658"
typedef enum status
{
    blocked,
    active,
    idle
} Status;
typedef struct _User
{
    char *username;
    char *password;
    Status status;
    char errCount;
} * User;

User newUser(char *username, char *password, Status status);
void blockUser(User user);
void activeUser(User user);
void setPassword(User user, char *password);
Status getUserStatus(User user);
char *getUserName(User user);
char *getUserPassword(User user);
void freeUser(User user);

Dllist makUsersList(char *filename);
User existsUser(Dllist list, char *username);
void addUser(Dllist list, User user);
void export(Dllist list, char *filename);
void freeUserList(Dllist list);
int activate(User user, char *activationCode);
User verify(Dllist list, char *username, char *password);
int checkPassword(User user, char *password);
int updatePassword(User user, char *password);
#endif