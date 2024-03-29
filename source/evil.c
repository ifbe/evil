#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#if (defined(_WIN32) || defined(__WIN32__))
#define mkdir(A, B) mkdir(A)
#endif
//
void filedata_create();
void filedata_delete();
void filemd5_create();
void filemd5_delete();
//
void funcdata_create();
void funcdata_delete();
void funcindex_create();
void funcindex_delete();
//
void strdata_create();
void strdata_delete();
void strhash_create();
void strhash_delete();
//
void relation_create();
void relation_delete();
//
void worker_create();
void worker_delete();
//
int disasm(int argc,char** argv);
int disasm_arm64(int argc,char** argv);
int disasm_x8664(int argc,char** argv);
int follow_arm64(int argc,char** argv);
int follow_x8664(int argc,char** argv);
int travel_arm64(int argc,char** argv);
int travel_x8664(int argc,char** argv);
//
int learn(int argc,char** argv);
int compile(int argc,char** argv);
//
int conv(int argc,char** argv);
int format(int argc,char** argv);
//
int create(int argc,char** argv);
int delete(int argc,char** argv);
int search(int argc,char** argv);
int modify(int argc,char** argv);
//
int render(int argc,char** argv);
int kirchhoff(int argc,char** argv);
int route(int argc,char** argv);
int serve(int argc,char** argv);
int substr(int argc,char** argv);
//
int mnist(int argc,char** argv);
int llama(int argc,char** argv);




void help(char* buf)
{
	if(buf != 0)printf("wrong usage: %s\n", buf);
	printf("analyze thing\n");
	printf("	a.exe disasm xxx.exe\n");
	printf("	a.exe follow xxx.bin\n");
	printf("	a.exe compile test.c\n");
	printf("	a.exe learn aaa.c /some/dir/bbb.cpp /my/folder/haha*\n");
	printf("easy operation\n");
	printf("	a.exe conv ac2intel test.ac\n");
	printf("	a.exe format test.c\n");
	printf("mydb operation\n");
	printf("	a.exe insert\n");
	printf("	a.exe delete\n");
	printf("	a.exe change\n");
	printf("	a.exe search str func@c0 file@20\n");
	printf("special operation\n");
	printf("	a.exe render\n");
	printf("	a.exe kirchhoff\n");
	printf("	a.exe route\n");
	printf("	a.exe serve\n");
	printf("	a.exe substr\n");
	printf("highlevel operation\n");
	printf("	a.exe quantization in.bin out.bin\n");
	printf("	OMP_NUM_THREADS=4 a.exe llama llama7b.bin tokenizer.bin\n");
}
int main(int argc, char** argv)
{
	if(argc==1){
		help(0);
		return 0;
	}


	//--------------------------.42 dir check------------------
	mkdir(".42", S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	mkdir(".42/chip", S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	mkdir(".42/file", S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	mkdir(".42/func", S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	mkdir(".42/pin", S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	mkdir(".42/point", S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	mkdir(".42/shape", S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	mkdir(".42/str", S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	mkdir(".42/wav", S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	mkdir(".42/wire", S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	//---------------------------------------------------------


	//-------------------------before--------------------------
	filemd5_create();
	filedata_create();

	funcindex_create();
	funcdata_create();

	strhash_create();
	strdata_create();

	relation_create();
	//----------------------------------------------------------


	//check
	if(argv[1][0] < 0x20){
		help(0);
		return 0;
	}

	//parse binary

	//parse assembly
	else if(strcmp(argv[1], "disasm_arm64") == 0){
		disasm_arm64(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "disasm_x8664") == 0){
		disasm_x8664(argc-1, argv+1);
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

	//parse c
	else if(strcmp(argv[1], "learn") == 0){
		learn(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "compile") == 0){
		compile(argc-1, argv+1);
	}

	//parse humanlang

	//operate simple
	else if(strcmp(argv[1], "conv") == 0){
		conv(argc-1, argv+1);
	}
	else if(strcmp(argv[1], "format") == 0){
		format(argc-1, argv+1);
	}

	//operate database
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

	//operate creative
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

	//operate highlevel
	else if(strncmp(argv[1], "mnist", 5) == 0){
		mnist(argc-1, argv+1);
	}
	else if(strncmp(argv[1], "llama", 5) == 0){
		llama(argc-1, argv+1);
	}

	//wrong usage
	else{
		help(argv[1]);
	}


	//-------------------------after---------------------------
	relation_delete();

	strhash_delete();
	strdata_delete();

	funcindex_delete();
	funcdata_delete();

	filemd5_delete();
	filedata_delete();
	//---------------------------------------------------------
	return 0;
}
