#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio_ext.h>
int main()
{
	int b = 0;
	int a = 0;
	if (fork() == 0)
	{
		printf("child: %d %p\n", --a, &a);
	}
	else
	{
		sleep(1);
		printf("parent: %d %p\n", ++b, &b);
	}
	return 0;
}