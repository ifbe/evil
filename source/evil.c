#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//lv0
int conv(int argc,char** argv);
int format(int argc,char** argv);
//lv1
int assembly_arm64(int argc,char** argv);
int assembly_x8664(int argc,char** argv);
int assembly_mips64(int argc,char** argv);
int assembly_riscv64(int argc,char** argv);
int disasm(int argc,char** argv);
int disasm_arm64(int argc,char** argv);
int disasm_x8664(int argc,char** argv);
int disasm_mips64(int argc,char** argv);
int disasm_riscv64(int argc,char** argv);
int follow_arm64(int argc,char** argv);
int follow_x8664(int argc,char** argv);
int travel_arm64(int argc,char** argv);
int travel_x8664(int argc,char** argv);
//lv2
int learn(int argc,char** argv);
int compile(int argc,char** argv);
int create(int argc,char** argv);
int delete(int argc,char** argv);
int search(int argc,char** argv);
int modify(int argc,char** argv);
int render(int argc,char** argv);
int kirchhoff(int argc,char** argv);
int route(int argc,char** argv);
int serve(int argc,char** argv);
int substr(int argc,char** argv);
//lv3
int mnist(int argc,char** argv);
int llama(int argc,char** argv);




void help(char* buf)
{
	//if(buf != 0)printf("wrong usage: %s\n", buf);
	printf("lv0:\n");
	printf("	a.exe conv ac2intel test.ac\n");
	printf("	a.exe format test.c\n");
	printf("lv1:\n");
	printf("	a.exe disasm xxx.exe\n");
	printf("	a.exe follow xxx.bin\n");
	printf("lv2:\n");
	printf("	a.exe learn aaa.c /some/dir/bbb.cpp /my/folder/haha*\n");
	printf("	a.exe compile test.c\n");
	printf("	a.exe insert\n");
	printf("	a.exe delete\n");
	printf("	a.exe change\n");
	printf("	a.exe search str func@c0 file@20\n");
	printf("	a.exe render\n");
	printf("	a.exe kirchhoff\n");
	printf("	a.exe route\n");
	printf("	a.exe serve\n");
	printf("	a.exe substr\n");
	printf("lv3:\n");
	printf("	a.exe mnist train model=xxx image=yyy label=zzz\n");
	printf("	a.exe mnist infer model=xxx image=yyy label=zzz\n");
	printf("	a.exe llama quanti in.bin out.bin\n");
	printf("	a.exe llama infer llama7b.bin tokenizer.bin\n");
}
int main(int argc, char** argv)
{
	if(argc==1){
		help(0);
		return 0;
	}
	if(argv[1][0] < 0x20){
		help(0);
		return 0;
	}

	//lv0
	else if(strcmp(argv[1], "conv") == 0){
		conv(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "format") == 0){
		format(argc-1, argv+1);
	}

	//lv1
	else if(strcmp(argv[1], "disasm_arm64") == 0){
		disasm_arm64(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "disasm_x8664") == 0){
		disasm_x8664(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "disasm_mips64") == 0){
		disasm_mips64(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "disasm_riscv64") == 0){
		disasm_riscv64(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "disasm") == 0){
		disasm(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "follow_arm64") == 0){
		follow_arm64(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "follow_x8664") == 0){
		follow_x8664(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "travel_arm64") == 0){
		travel_arm64(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "travel_x8664") == 0){
		travel_x8664(argc-1, argv+1);
	}
	else if(strncmp(argv[1], "asm_arm64", 9) == 0){
		assembly_arm64(argc-1, argv+1);
	}
	else if(strncmp(argv[1], "asm_x8664", 9) == 0){
		assembly_x8664(argc-1, argv+1);
	}
	else if(strncmp(argv[1], "asm_mips64", 9) == 0){
		assembly_mips64(argc-1, argv+1);
	}
	else if(strncmp(argv[1], "asm_riscv64", 9) == 0){
		assembly_riscv64(argc-1, argv+1);
	}

	//lv2
	else if(strcmp(argv[1], "learn") == 0){
		learn(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "compile") == 0){
		compile(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "create") == 0){
		create(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "delete") == 0){
		delete(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "search") == 0){
		search(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "modify") == 0){
		modify(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "render") == 0){
		render(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "kirchhoff") == 0){
		kirchhoff(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "route") == 0){
		route(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "serve") == 0){
		serve(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "substr") == 0){
		substr(argc-1, argv+1);
	}

	//lv3
	else if(strncmp(argv[1], "mnist", 5) == 0){
		mnist(argc-1, argv+1);
	}
	else if(strncmp(argv[1], "llama", 5) == 0){
		llama(argc-1, argv+1);
	}

	//
	else{
		help(argv[1]);
	}

	return 0;
}
