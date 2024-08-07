#include<stdio.h>
#include<stdlib.h>
#include<string.h>
int llama_info(int argc, char** argv);
int llama_quanti(int argc, char** argv);
int llama_infer(int argc, char** argv);


int llama(int argc, char** argv)
{
	if(argc < 2){
		printf("./a.exe llama quan /path/to/model.bf16 /path/to/tokenizer.bin\n");
		printf("./a.exe llama infer /path/to/model.bf16 /path/to/tokenizer.bin\n");
	}
	else if(0 == strncmp("info", argv[1], 4)){
		llama_info(argc-1, &argv[1]);
	}
	else if(0 == strncmp("quanti", argv[1], 6)){
		llama_quanti(argc-1, &argv[1]);
	}
	else if(0 == strncmp("infer", argv[1], 5)){
		llama_infer(argc-1, &argv[1]);
	}
	return 0;
}
