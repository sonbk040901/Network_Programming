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
typedef struct user
{
    char *username;
    char *password;
    Status status;
    char *homepage;
} * User;
User newUser(char *username, char *password, char *homepage, Status status); // done
void blockUser(User user);                                                   // done
void activeUser(User user);                                                  // done
void setPassword(User user, char *password);                                 // done
Status getUserStatus(User user);                                             // done
char *getUserName(User user);                                                // done
char *getUserPassword(User user);                                            // done
void freeUser(User user);                                                    // done

Dllist makUsersList(char *filename);                      // done
User existsUser(Dllist list, char *username);             // done
void addUser(Dllist list, User user);                     // done
void exportFile(Dllist list, char *filename);             // done
void freeUserList(Dllist list);                           // done
int activate(User user, char *activationCode);            // done
User verify(Dllist list, char *username, char *password); // done
int checkPassword(User user, char *password);             // done
#endif