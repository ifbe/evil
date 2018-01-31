#include<stdio.h>
#include<string.h>
void readthemall(int);
int search_one(void*, int, void*, int);




unsigned char str[0x1000];
unsigned char buf[0x100000];
void serve(int argc, char** argv)
{
	int j;
	readthemall(1);

	while(1)
	{
		printf("->");
		fgets(str, 0x1000, stdin);
		for(j=0;j<0x1000;j++)
		{
			if(str[j] <= 0xa)
			{
				str[j] = 0;
				break;
			}
		}
		if((str[0] == 'q')&&(str[1] == 0))break;

		j = search_one(buf, 0x100000, str, strlen(str));
		printf("%.*s\n", j, buf);
	}
}
