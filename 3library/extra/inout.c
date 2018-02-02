#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define u8 unsigned char




int fixarg(int argc, char** argv)
{
	
}
int input(u8* buf, int len)
{
	int j;
	fgets(buf, 0x1000, stdin);
	for(j=0;j<0x1000;j++)
	{
		if(buf[j] <= 0xa)
		{
			buf[j] = 0;
			break;
		}
	}
	return strlen(buf);
}
int output()
{
	
}