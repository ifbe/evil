#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//
void filedata_create();
void filedata_delete();
void filetrav_create();
void filetrav_delete();
//
void funcdata_create();
void funcdata_delete();
void funcindx_create();
void funcindx_delete();
//
void stringdata_create();
void stringdata_delete();
void stringhash_create();
void stringhash_delete();
//
void connect_create();
void connect_delete();
//
void worker_create();
void worker_delete();
//
int learn(int argc,char** argv);
int check(int argc,char** argv);
int hash(int argc,char** argv);
int search(int argc,char** argv);
int change(int argc,char** argv);
int create(int argc,char** argv);
int delete(int argc,char** argv);




void help()
{
	printf("a.exe\n{\n");
	printf("#step1: learning\n");
	printf("	a.exe learn aaa.c /some/dir/bbb.cpp /my/folder/haha*\n\n");
	printf("#step2: thinking\n");
	printf("	a.exe check aaa.seed\n");
	printf("	a.exe hash bbb.seed\n\n");
	printf("#step3: working\n");
	printf("	a.exe search name\n");
	printf("	a.exe change oldname newname\n");
	printf("	a.exe create /main/f1/f2/ name\n");
	printf("	a.exe delete name\n\n");
	printf("#step4:\n");
	printf("}\n");
}
int main(int argc, char** argv)
{
	//--------------------------help---------------------------
	if(argc==1)
	{
		help();
		return 0;
	}
	//---------------------------------------------------------




	//-------------------------before--------------------------
	filetrav_create();
	filedata_create();

	funcindx_create();
	funcdata_create();

	stringhash_create();
	stringdata_create();

	connect_create();
	worker_create();
	//----------------------------------------------------------




	//auto mode
	if(strcmp(argv[1] , "learn") == 0)
	{
		learn(argc-1 , argv+1);
	}

	//
	else if(strcmp(argv[1] , "check") == 0)
	{
		check(argc-1 , argv+1);
	}

	//
	else
	{
		help();
		return 0;
	}




	//-------------------------after---------------------------
	worker_delete();
	connect_delete();

	stringhash_delete();
	stringdata_delete();

	funcindx_delete();
	funcdata_delete();

	filetrav_delete();
	filedata_delete();
	//---------------------------------------------------------
}
