#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int mnist_infer(int argc, char** argv);
int mnist_train(int argc, char** argv);


int mnist(int argc, char** argv)
{
	if(argc < 2){
		printf("./a.exe mnist train /path/to/image /path/to/label\n");
		printf("./a.exe mnist infer /path/to/image /path/to/label\n");
	}
	else if(0 == strncmp("infer", argv[1], 5)){
		mnist_infer(argc-1, &argv[1]);
	}
	else if(0 == strncmp("train", argv[1], 6)){
		mnist_train(argc-1, &argv[1]);
	}
	return 0;
}
