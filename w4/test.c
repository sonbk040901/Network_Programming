#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
int main(int argc, char const *argv[])
{
    char str[100];
    strcpy(str, argv[1]);
    char *token = strtok(str, ",");
    char strs[3][100];
    int type = atoi(token);
    int num = type == 1 ? 2 : 1;
    for (int i = 0; i < num && token != NULL; i++)
    {
        token = strtok(NULL, ",");
        strcpy(strs[i], token);
    }
    for (int i = 0; i < 3; i++)
    {
        puts(strs[i]);
    }

    return 0;
}
