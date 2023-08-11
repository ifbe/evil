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


void gbk2utf(u8* dst, u8* src)
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
		//printf("uni=%x\n", tmp);

		ret = 0;
		WideCharToMultiByte(
			CP_UTF8, 0,
			(void*)&tmp, -1,
			(void*)&ret, 4,
			NULL, NULL
		);
		//printf("utf=%x\n", ret);

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

	int ret;
	DWORD num;
	INPUT_RECORD irInBuf[2];
	KEY_EVENT_RECORD k0, k1;
	while(1)
	{
		if(!ReadConsoleInput(hStdin, irInBuf, 1, &num))
		{
			printf("ReadConsoleInput");
			return 0;
		}
		if(KEY_EVENT != irInBuf[0].EventType)continue;

		k0 = irInBuf[0].Event.KeyEvent;
/*
printf("{num=%x,bKeyDown=%x,wRepeatCount=%x,wVirtualKeyCode=%x,wVirtualScanCode=%x,UnicodeChar=%x,dwControlKeyState=%x}\n",
num,
k0.bKeyDown, k0.wRepeatCount,
k0.wVirtualKeyCode, k0.wVirtualScanCode,
k0.uChar.UnicodeChar, k0.dwControlKeyState
);
*/
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
		else if(k0.uChar.UnicodeChar < 0x80)
		{
			if( (4 == k0.uChar.UnicodeChar) && (k0.dwControlKeyState&8) )return 0;
			return k0.uChar.UnicodeChar;
		}
		else{
			if(!ReadConsoleInput(hStdin, &irInBuf[1], 1, &num)){
				printf("read unicode error\n");
				return 0;
			}
			k1 = irInBuf[1].Event.KeyEvent;
			//k2 = irInBuf[2].Event.KeyEvent;
			//printf("k0=%x,k1=%x,k2=%x\n",k0.uChar.UnicodeChar,k1.uChar.UnicodeChar,k2.uChar.UnicodeChar);
			return (k0.uChar.UnicodeChar&0xff) | ((k1.uChar.UnicodeChar&0xff) << 8);
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
		//printf("<%x>",ret);
		if(0 == ret)break;
		else if(0x8 == ret){
			if(0 == j)continue;

			if(buf[j-1] < 0x80){
				printf("\b \b");
				j--;
			}
			else{
				printf("\b\b  \b\b");
				j -= 3;
			}
			buf[j] = 0;
		}
		else if(0xd >= ret){
			buf[j] = 0;
			j++;
			printf("\n");
			break;
		}
		else if(0x80 > ret){
			buf[j] = ret;
			j++;
			printf("%c", ret);
		}
		else if(0x1b == (ret&0xff)){
			printf("%x\n",ret);
		}
		else{
			printf("%s", &ret);
			//printf("gbk=%x\n", ret);

			tmp = 0;
			MultiByteToWideChar(
				CP_ACP, 0,
				(void*)&ret, -1,
				(void*)&tmp, 2
			);
			//printf("unicode=%x\n", tmp);

			ret = 0;
			WideCharToMultiByte(
				CP_UTF8, 0,
				(void*)&tmp, -1,
				(void*)&ret, 4,
				NULL, NULL
			);
			//printf("utf8=%x\n", ret);

			*(u32*)(buf+j) = ret;
			for(ret=0;ret<4;ret++){
				if(buf[j] >= 0xd)j++;
			}
		}
	}

	//cond 0: EOF
	//cond 1: 0xa
	//cond 2: 1 2 0xa
	return j;
}


#else


void output(char* buf, int len)
{
	printf("%.*s\n", len, buf);
}
int input(u8* buf, int len)
{
	char* ret = fgets((void*)buf, 0x1000, stdin);
	if(0 == ret)return 0;

	int j;
	for(j=0;j<0x1000;j++){
		if(buf[j] <= 0xd){
			buf[j] = 0;
			j++;
			break;
		}
	}
	return j;
}


#endif
