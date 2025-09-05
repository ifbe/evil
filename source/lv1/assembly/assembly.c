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

#define CPU_X8664 0
#define CPU_ARM64 1
#define CPU_MIPS64 2
#define CPU_RISCV64 3
struct offlen{
	u8 off;
	u8 len;
}__attribute__((packed));
void assembly_compile_x8664(u8* buf, int len, struct offlen* tab, int cnt);
void assembly_compile_arm64(u8* buf, int len, struct offlen* tab, int cnt);
void assembly_compile_mips64(u8* buf, int len, struct offlen* tab, int cnt);
void assembly_compile_riscv64(u8* buf, int len, struct offlen* tab, int cnt);




int issym(u8 key){
	switch(key){
	case '=':
	case '+':
	case '-':
	case '*':
	case '/':
	case '&':
	case '|':
	case '^':
		return 1;
	}
	return 0;
}
int iskuohao(u8 key){
	switch(key){
	case '(':
	case ')':
	case '[':
	case ']':
	case '{':
	case '}':
		return 1;
	}
	return 0;
}
int iscomma(u8 key){
	switch(key){
	case ',':
	case ';':
		return 1;
	}
	return 0;
}
void printtoken(u8* buf, int len){
	printf(" %.*s ", len, buf);
}
void assembly_oneline(u8* buf, int len, int cpu)
{
	if(len < 2)return;
	printf("--------");

	int cnt = 0;
	struct offlen tab[16];

	int j;
	int k = -1;
	for(j=0;j<=len;j++){
		if( (buf[j] == ' ') | (buf[j] ==  '\t') | (j==len) ){
			if(k >= 0){
				printtoken(buf+k, j-k);
				tab[cnt].off = k;
				tab[cnt].len = j-k;
				cnt += 1;
			}
			k = -1;
		}
		else if(iscomma(buf[j])){
			if(k < 0)k = j;
			else{
				printtoken(buf+k, j-k);
				tab[cnt].off = k;
				tab[cnt].len = j-k;
				cnt += 1;
				k = j;
			}
		}
		else if(iskuohao(buf[j])){
			if(k < 0)k = j;
			else{
				printtoken(buf+k, j-k);
				tab[cnt].off = k;
				tab[cnt].len = j-k;
				cnt += 1;
				k = j;
			}
		}
		else if(issym(buf[j])){
			if(k < 0)k = j;
			else{
				if(issym(buf[k]))continue;
				else{
					printtoken(buf+k, j-k);
					tab[cnt].off = k;
					tab[cnt].len = j-k;
					cnt += 1;
					k = j;
				}
			}
		}
		else{
			if(k < 0)k = j;
			else if( issym(buf[k]) | iskuohao(buf[k]) | iscomma(buf[k]) ){
				printtoken(buf+k, j-k);
				tab[cnt].off = k;
				tab[cnt].len = j-k;
				cnt += 1;
				k = j;
			}
		}
	}
	printf("--------\n");

	switch(cpu){
	case CPU_X8664:
		assembly_compile_x8664(buf,len, tab,cnt);
		break;
	case CPU_ARM64:
		assembly_compile_arm64(buf,len, tab,cnt);
		break;
	case CPU_MIPS64:
		assembly_compile_mips64(buf,len, tab,cnt);
		break;
	case CPU_RISCV64:
		assembly_compile_riscv64(buf,len, tab,cnt);
		break;
	}
	printf("\n");
}
void parseassembly(u8* buf, int len, int cpu)
{
	printf("len=%d\n", len);
	int j,k=0;
	for(j=0;j<=len;j++){
		if( ('\n' == buf[j]) | (j==len) ){
			//printf("%.*s\n", j-k, buf+k);
			assembly_oneline(buf+k, j-k, cpu);
			k = j+1;
		}
	}
}
void assembly_arm64(int argc, char** argv)
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

	parseassembly(buf, ret, CPU_ARM64);

release:
	free(buf);
theend:
	close(fd);
}
void assembly_x8664(int argc, char** argv)
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

	parseassembly(buf, ret, CPU_X8664);

release:
	free(buf);
theend:
	close(fd);
}
void assembly_mips64(int argc, char** argv)
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

	parseassembly(buf, ret, CPU_MIPS64);

release:
	free(buf);
theend:
	close(fd);
}
void assembly_riscv64(int argc, char** argv)
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

	parseassembly(buf, ret, CPU_RISCV64);

release:
	free(buf);
theend:
	close(fd);
}