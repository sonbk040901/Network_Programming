#include "user.h"

User newUser(char *username, char *password, Status status)
{
    User user = malloc(sizeof(struct _User));
    user->username = strdup(username);
    user->password = strdup(password);
    user->status = status;
    user->errCount = status == blocked ? 3 : 0;
    return user;
}
void blockUser(User user)
{
    user->status = blocked;
}
void activeUser(User user)
{
    user->status = active;
}
void setPassword(User user, char *password)
{
    free(user->password);
    user->password = strdup(password);
}
Status getUserStatus(User user)
{
    return user->status;
}
void freeUser(User user)
{
    free(user->username);
    free(user->password);
    free(user);
}

char *getUserName(User user)
{
    return user->username;
}
char *getUserPassword(User user)
{
    return user->password;
}
Dllist makUsersList(char *filename)
{
    Dllist list = new_dllist();
    IS is = new_inputstruct(filename);
    if (is == NULL)
    {
        perror(filename);
        exit(EXIT_FAILURE);
    }
    while (get_line(is) >= 0)
    {
        dll_append(list, new_jval_v(newUser(is->fields[0], is->fields[1], atoi(is->fields[2]))));
    }
    jettison_inputstruct(is);
    return list;
}
User existsUser(Dllist list, char *username)
{
    Dllist root;
    User user;
    dll_traverse(root, list)
    {
        user = (User)jval_v(dll_val(root));
        if (strcmp(user->username, username) == 0)
        {
            return user;
        }
    }
    return (User)0;
}
void addUser(Dllist list, User user)
{
    dll_append(list, new_jval_v(user));
}
void export(Dllist list, char *filename)
{
    Dllist temp;
    FILE *fout = fopen(filename, "w+");
    dll_traverse(temp, list)
    {
        User user = (User)jval_v(dll_val(temp));
        // printf("User: %s %s %d\n", user->username, user->password, user->status);
        fprintf(fout, "%s %s %d", user->username, user->password, user->status);
        if (temp != dll_last(list))
        {
            fprintf(fout, "\n");
        }
    }
    fclose(fout);
}
void freeUserList(Dllist list)
{
    Dllist temp;
    dll_traverse(temp, list)
    {
        User user = (User)jval_v(dll_val(temp));
        freeUser(user);
    }
}
int activate(User user, char *activationCode)
{
    if (strcmp(activationCode, ACTIVATION_CODE) != 0)
    {
        return 0;
    }
    else
    {
        activeUser(user);
        return 1;
    }
}
User verify(Dllist list, char *username, char *password)
{
    Dllist root;
    User user;
    dll_traverse(root, list)
    {
        user = (User)jval_v(dll_val(root));
        if (strcmp(username, user->username) == 0)
        {
            if (strcmp(password, user->password) == 0)
            {
                return user;
            }
            else
            {
                return (User)0;
            }
        }
    }
    return (User)0;
}
int checkPassword(User user, char *password)
{
    return strcmp(password, user->password) == 0;
}
int updatePassword(User user, char *password)
{
    free(user->password);
    user->password = strdup(password);
    return 1;
}