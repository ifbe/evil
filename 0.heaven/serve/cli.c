#include<stdio.h>
#include<string.h>
void readall(int);
void search_one(char*, int);




char buf[0x1000];
void serve(int argc, char** argv)
{
	int j;
	readall(1);

	while(1)
	{
		printf("->");
		fgets(buf, 0x1000, stdin);
		for(j=0;j<0x1000;j++)
		{
			if(buf[j] <= 0xa)
			{
				buf[j] = 0;
				break;
			}
		}
		if((buf[0] == 'q')&&(buf[1] == 0))break;

		search_one(buf, strlen(buf));
	}
}