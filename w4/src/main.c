#include <stdio.h>
int main(int argc, char const *argv[])
{
	printf("Hello World!\n");
	perror("errr");
	exit(1);
	return (0);
}