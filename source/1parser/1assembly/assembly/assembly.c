#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<errno.h>
#include<sys/stat.h>
#include<sys/types.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define s8 signed char
#define s16 signed short
#define s32 signed int
#define s64 signed long long
#ifndef O_BINARY
	#define O_BINARY 0x0
#endif
int hexstr2u32(void* str, void* dat);

struct offlen{
	u8 off;
	u8 len;
}__attribute__((packed));
void assembly_x8664(u8* buf, int len, struct offlen* tab, int cnt);




void assembly_oneline(u8* buf, int len)
{
	int cnt = 0;
	struct offlen tab[16];

	int j;
	int k = -1;
	for(j=0;j<=len;j++){
		if( (buf[j] == ' ') | (buf[j] ==  '\t') | (j==len) ){
			if(k >= 0){
				printf("(%.*s)", j-k, buf+k);
				tab[cnt].off = k;
				tab[cnt].len = j-k;
				cnt += 1;
			}
			k = -1;
		}
		else if( (buf[j] == '=') | (buf[j] == '+') ){
			if(k >= 0){
				printf("(%.*s)", j-k, buf+k);
				tab[cnt].off = k;
				tab[cnt].len = j-k;
				cnt += 1;
			}
			printf("[%c]", buf[j]);
			tab[cnt].off = j;
			tab[cnt].len = 1;
			cnt += 1;

			k = -1;
		}
		else{
			if(k < 0)k = j;
		}
	}
	printf("\n");

	assembly_x8664(buf,len, tab,cnt);
}
void parseassembly(u8* buf, int len)
{
	printf("len=%d\n", len);
	int j,k=0;
	for(j=0;j<len;j++){
		if('\n' == buf[j]){
			//printf("%.*s\n", j-k, buf+k);
			assembly_oneline(buf+k, j-k);
			k = j+1;
		}
	}
}
void assembly(int argc, char** argv)
{
	u32 at = 0;
	u32 sz = 0x100000;
	if(argc < 2)return;

	int fd = open(argv[1] , O_RDONLY|O_BINARY);
	if(fd <= 0){
		printf("errno=%d@open\n", errno);
		return;
	}

	u8* buf = malloc(sz);
        if(0 == buf){
		printf("errno=%d@malloc\n", errno);
		goto theend;
	}

	int ret = lseek(fd, at, SEEK_SET);
	if(ret < 0){
		printf("errno=%d@lseek\n", errno);
		goto release;
	}

	ret = read(fd, buf, sz);
	if(ret <= 0){
		printf("errno=%d@read\n", errno);
		goto release;
	}

	parseassembly(buf, ret);

release:
	free(buf);
theend:
	close(fd);
}
