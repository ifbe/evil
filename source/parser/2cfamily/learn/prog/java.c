#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#ifndef O_BINARY
	//mingw64 compatiable
	#define O_BINARY 0x0
#endif




//count
static int countbyte=0;
static int countline=0;
//
static int innote = 0;
#define INNOTE_SINGLELINE 1
#define INNOTE_MULTILINE 2
static int instr = 0;




static int java_read(u8* buf, int len)
{
	unsigned char ch=0;
	countbyte = 0;

	for(;countbyte<0x100000;countbyte++){
		ch = buf[countbyte];

		if(countbyte > len){
			break;
		}

		if(0 == ch){
			break;
		}

		else if( (0xa == ch) | (0xd == ch) ){
			countline++;
			if( (0xd == ch) && (0xa == buf[countbyte+1]) )countbyte++;

			instr = 0;
			if(innote == INNOTE_SINGLELINE)innote = 0;
		}

		else if('\\' == ch){
			if(0xa == buf[countbyte+1])countline++;

			else if(0xd == buf[countbyte+1]){
				if(0xa == buf[countbyte+2])countbyte++;
				countline++;
			}

			countbyte++;
			continue;
		}

		else if('/' == ch){
			if(innote>0|instr>0)continue;

			if('/' == buf[countbyte+1]){
				innote = INNOTE_SINGLELINE;
				countbyte++;
			}

			else if('*' == buf[countbyte+1]){
				innote = INNOTE_MULTILINE;
				countbyte++;
			}
		}

		else if('*' == ch){
			if((innote==INNOTE_SINGLELINE)|(instr>0))continue;

			if('/' == buf[countbyte+1]){
				if(innote == INNOTE_MULTILINE){
					innote = 0;
					countbyte++;
				}
			}
		}

		else if('\"' == ch){
			if(innote>0)continue;
			if(instr == 0){
				instr = countbyte+1;
			}
			else{
				printf("\"%.*s\"\n", countbyte-instr, buf+instr);
				//purec_str(buf+instr, countbyte-instr);
				instr=0;
			}
		}

		else if('\'' == ch){
			if(innote>0|instr>0)continue;

			if('\'' == buf[countbyte+2]){
				countbyte += 2;
			}
		}
	}

	//printf("byte=%x,line=%d\n",countbyte,countline);
	return 0;
}
static void java_write()
{
}
static void java_stop()
{
/*
	printf("@%x@%d -> %d,%d,%d,%d\n\n\n\n\n",
		countbyte,
		countline,
		infunc,
		inmarco,
		innote,
		instr
	);
*/
	//write(outfile,"\n\n\n\n",4);
}
static void java_start()
{
	countbyte = countline = 0;
}




void java_delete()
{
}
void java_create(u64* that, u64* this)
{
	this[0] = 0x6573726170;
	this[1] = hex32('j','a','v','a');
	this[4] = (u64)java_start;
	this[5] = (u64)java_stop;
	this[6] = (u64)java_read;
	this[7] = (u64)java_write;
}
