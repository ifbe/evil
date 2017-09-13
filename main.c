#include <stdio.h>
#include <stdlib.h>
#include <string.h>
void hash_create();
void hash_delete();
void string_create();
void string_delete();
void traverse_create();
void traverse_delete();
void worker_create();
void worker_delete();
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
int main(int argc,char *argv[])  
{
	//--------------------------help---------------------------
	if(argc==1)
	{
		help();
		return 0;
	}
	//---------------------------------------------------------




	//-------------------------before--------------------------
	hash_create();
	string_create();
	traverse_create();
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
		printf("@check\n");
	}
	else if(strcmp(argv[1] , "hash") == 0)
	{
		hash(argc-1 , argv+1);
	}

	//
	else if(strcmp(argv[1] , "create") == 0)
	{
		printf("@create\n");
	}
	else if(strcmp(argv[1] , "delete") == 0)
	{
		printf("@delete\n");
	}
	else if(strcmp(argv[1] , "search") == 0)
	{
		printf("@search\n");
	}
	else if(strcmp(argv[1] , "change") == 0)
	{
		printf("@change\n");
	}

	//
	else
	{
		help();
		return 0;
	}




	//-------------------------after---------------------------
	worker_delete();
	traverse_delete();
	string_delete();
	hash_delete();
	//---------------------------------------------------------
}
