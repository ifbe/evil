#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdarg.h>




#if (defined(_WIN32) || defined(__WIN32__))
#include<conio.h>
#include<windows.h>


void fixarg(u8* dst, u8* src)
{
	u32 ret,tmp;
	while(1)
	{
		if(*src < 0xa)break;
		if(*src < 0x80)
		{
			*dst = *src;
			dst++;
			src++;
			continue;
		}

		ret = *(u16*)src;
		src += 2;

		tmp = 0;
		MultiByteToWideChar(
			CP_ACP, 0,
			(void*)&ret, -1,
			(void*)&tmp, 2
		);
		//printf("%x\n", tmp);

		ret = 0;
		WideCharToMultiByte(
			CP_UTF8, 0,
			(void*)&tmp, -1,
			(void*)&ret, 4,
			NULL, NULL
		);
		//printf("%x\n", ret);

		*(u32*)dst = ret;
		for(ret=0;ret<4;ret++)
		{
			if(*dst >= 0xa)dst++;
		}
	}
	*dst = 0;
}
void output(u8* buf, int len)
{
	int i,j,k=0;
	for(j=0;j<len;j++)
	{
		//printf("(%02x)",buf[j]);
		if(buf[j] < 0x80)
		{
			printf("%c", buf[j]);
		}
		else
		{
			char utf8[8];
			char unicode[8];
			char gbk[8];

			if(buf[j] < 0xe0)i = 2;
			else if(buf[j] < 0xf0)i = 3;
			else if(buf[j] < 0xf8)i = 4;
			else if(buf[j] < 0xfc)i = 5;
			else if(buf[j] < 0xfe)i = 6;

			utf8[i] = 0;
			for(k=0;k<i;k++)utf8[k] = buf[j+k];
			j = j+i-1;

			MultiByteToWideChar(CP_UTF8, 0, utf8, -1, (void*)unicode, 4);
			WideCharToMultiByte(CP_ACP, 0, (void*)unicode, -1, gbk, 4, NULL, NULL);
			printf("%s",gbk);
		}
	}
}
int lowlevel_input()
{
	DWORD num;
	int ret,tmp;
	INPUT_RECORD irInBuf[2];
	KEY_EVENT_RECORD k0, k1;
	HANDLE hStdin, hStdout;

	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	if(INVALID_HANDLE_VALUE == hStdin)
	{
		printf("error@hStdin\n");
		exit(-1);
	}

	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	if(INVALID_HANDLE_VALUE == hStdout)
	{
		printf("error@hStdout\n");
		exit(-1);
	}

	while(1)
	{
		if(!ReadConsoleInput(hStdin, irInBuf, 2, &num))
		{
			printf("ReadConsoleInput");
			return 0;
		}
		if(KEY_EVENT != irInBuf[0].EventType)continue;

		k0 = irInBuf[0].Event.KeyEvent;
		if(0 == k0.bKeyDown)continue;

		if(k0.uChar.AsciiChar == 0)
		{
			ret = k0.wVirtualKeyCode;

			if(ret == 0x26)return 0x415b1b;
			else if(ret == 0x28)return 0x425b1b;
			else if(ret == 0x27)return 0x435b1b;
			else if(ret == 0x25)return 0x445b1b;
			else
			{
				//printf("kbd:%x\n", ret);
				continue;
			}
		}
		else
		{
			ret = k0.uChar.UnicodeChar;
			if(ret < 0x80)return ret;

			k1 = irInBuf[1].Event.KeyEvent;
			return ret | (k1.uChar.UnicodeChar << 8);
		}
	}
	return 0;
}
int input(u8* buf, int len)
{
	int j, ret, tmp;

	j = 0;
	while(1)
	{
		ret = lowlevel_input();
		if(0x8 == ret)
		{
			if(0 != j)
			{
				if(buf[j-1] < 0x80)
				{
					printf("\b \b");
					j--;
				}
				else
				{
					printf("\b\b  \b\b");
					j -= 3;
				}
				buf[j] = 0;
			}
		}
		else if(0xd >= ret)break;
		else if(0x80 > ret)
		{
			printf("%c", ret);
		}
		else
		{
			printf("%s", (void*)&ret);

			tmp = 0;
			MultiByteToWideChar(
				CP_ACP, 0,
				(void*)&ret, -1,
				(void*)&tmp, 2
			);
			//printf("%x\n", tmp);

			ret = 0;
			WideCharToMultiByte(
				CP_UTF8, 0,
				(void*)&tmp, -1,
				(void*)&ret, 4,
				NULL, NULL
			);
			//printf("%x\n", ret);
		}

		*(u32*)(buf+j) = ret;
		for(ret=0;ret<4;ret++)
		{
			if(buf[j] >= 0xd)j++;
		}
	}
	buf[j] = 0;
	return j;
}


#else


void output(char* buf, int len)
{
	printf("%.*s\n", len, buf);
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


#endif