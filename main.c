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
void connect_create();
void connect_delete();
//
void worker_create();
void worker_delete();
//
int learn(int argc,char** argv);
int think(int argc,char** argv);
//
int search(int argc,char** argv);
int delete(int argc,char** argv);




void help()
{
	printf("usage:\n");
	printf("a.exe learn aaa.c /some/dir/bbb.cpp /my/folder/haha*\n");
	printf("a.exe search string func@c0 file@20\n");
	printf("a.exe think\n");
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




	//--------------------------.42 dir check------------------
	//{
	//struct stat s;
	//int err=stat(".42",&s);
	//if((-1==err) || !S_ISDIR(s.st_mode))
	//{
	//}
	//}
	mkdir(".42", S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	mkdir(".42/wire", S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	mkdir(".42/filemd5", S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	mkdir(".42/funcindex", S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	mkdir(".42/strhash", S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	mkdir(".42/filedata", S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	mkdir(".42/funcdata", S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	mkdir(".42/strdata", S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH);
	//---------------------------------------------------------




	//-------------------------before--------------------------
	filemd5_create();
	filedata_create();

	funcindex_create();
	funcdata_create();

	strhash_create();
	strdata_create();

	connect_create();
	worker_create();
	//----------------------------------------------------------




	//
	if(strcmp(argv[1] , "learn") == 0)
	{
		learn(argc-1 , argv+1);
	}
	else if(strcmp(argv[1] , "think") == 0)
	{
		think(argc-1 , argv+1);
	}
	else if(strcmp(argv[1] , "search") == 0)
	{
		search(argc-1 , argv+1);
	}
	else if(strcmp(argv[1] , "delete") == 0)
	{
		delete(argc-1 , argv+1);
	}
	else
	{
		help();
		return 0;
	}




	//-------------------------after---------------------------
	worker_delete();
	connect_delete();

	strhash_delete();
	strdata_delete();

	funcindex_delete();
	funcdata_delete();

	filemd5_delete();
	filedata_delete();
	//---------------------------------------------------------
}
