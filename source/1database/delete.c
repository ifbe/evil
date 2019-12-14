#include<stdio.h>




void delete(int argc, char** argv)
{
	int j;
	for(j=1;j<argc;j++)
	{
		printf("%s\n", argv[j]);
	}
}
