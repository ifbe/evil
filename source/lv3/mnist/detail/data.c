#include<stdio.h>
#include<stdlib.h>
#include<string.h>
typedef unsigned char u8;


FILE* imagefile = 0;
u8* imagemem = 0;
int mnist_image_open(char* path)
{
	imagefile = fopen(path, "rb");
	if(0 == imagefile)return -1;

	imagemem = (u8*)malloc(0x1000);
	if(0 == imagemem)return -2;

	return 0;
}
void mnist_image_close()
{
	if(imagemem){
		free(imagemem);
		imagemem = 0;
	}
	if(imagefile){
		fclose(imagefile);
		imagefile = 0;
	}
}
void* mnist_image_get(int id)
{
	int ret1 = fseek(imagefile, 16+id*28*28, SEEK_SET);
	if(ret1 != 0)return 0;

	int ret2 = fread(imagemem, 28*28, 1, imagefile);
	if(ret2 <= 0)return 0;

	return imagemem;
	//return &imagemem[16+id*28*28];
}


FILE* labelfile = 0;
u8* labelmem = 0;
int mnist_label_open(char* path)
{
	labelfile = fopen(path, "rb");
	if(0 == labelfile)return -1;

	labelmem = (u8*)malloc(0x1000);
	if(0 == labelmem)return -2;

	return 0;
}
void mnist_label_close()
{
	if(labelmem){
		free(labelmem);
		labelmem = 0;
	}
	if(labelfile){
		fclose(labelfile);
		labelfile = 0;
	}
}
void* mnist_label_get(int id)
{
	fseek(labelfile, 8+id, SEEK_SET);
	fread(labelmem, 1, 1, labelfile);
	return labelmem;
	//return &labelmem[8+id];
}



struct mnist_runstate{
	char* modelpath;
	float* modelmem;

	char* imagepath;
	char* labelpath;
};


FILE* modelfile = 0;
float* modelmem = 0;
int mnist_model_malloc(struct mnist_runstate* rs)
{
	modelmem = (float*)malloc(sizeof(float)*(28*28*10 + 10));
	if(0 == modelmem)return -2;

	rs->modelmem = modelmem;
	return 0;
}
int mnist_model_random()
{
	int j;
	for(j=0;j<28*28*10 + 10;j++)modelmem[j] = 0.0;
	return 0;
}
int mnist_model_load(char* path)
{
	modelfile = fopen(path, "rb");
	if(0 == modelfile)goto err1;

	int ret = fread(modelmem, sizeof(float)*(28*28*10 + 10), 1, modelfile);
	if(ret <= 0){
		printf("mnist_model_load fread ret=%d\n", ret);
		goto err2;
	}

success:
	fclose(modelfile);
	return 0;

err2:
	fclose(modelfile);
err1:
	mnist_model_random();
	return -2;
}
int mnist_model_save(char* path)
{
	modelfile = fopen(path, "wb");
	if(!modelfile){
		printf("mnist_model_save fopen ret=0\n");
		return -1;
	}

	int ret = fwrite(modelmem, sizeof(float)*(28*28*10 + 10), 1, modelfile);
	if(ret <= 0){
		printf("mnist_model_save fwrite ret=%d\n", ret);
		goto err1;
	}

success:
	fclose(modelfile);
	return 0;

err1:
	fclose(modelfile);
	return -1;
}


int mnist_init(int argc, char** argv, struct mnist_runstate* rs)
{
	rs->modelpath = 0;
	rs->imagepath = 0;
	rs->labelpath = 0;

	int j;
	for(j=1;j<argc;j++){
		if(0 == strncmp(argv[j], "model=", 6))rs->modelpath = argv[j]+6;
		if(0 == strncmp(argv[j], "image=", 6))rs->imagepath = argv[j]+6;
		if(0 == strncmp(argv[j], "label=", 6))rs->labelpath = argv[j]+6;
	}

	if(!rs->imagepath){
		return 0;
	}
	if(!rs->labelpath){
		return 0;
	}
	if(!rs->modelpath){
		rs->modelpath = "mnist.bin";
	}

	printf("modelpath=%s\n", rs->modelpath);
	printf("imagepath=%s\n", rs->imagepath);
	printf("labelpath=%s\n", rs->labelpath);

	int ret1 = mnist_image_open(rs->imagepath);
	if(ret1<0)goto err1;
	printf("image: file=%p, mem=%p\n", imagefile, imagemem);

	int ret2 = mnist_label_open(rs->labelpath);
	if(ret2<0)goto err2;
	printf("label: file=%p, mem=%p\n", labelfile, labelmem);

	int ret3 = mnist_model_malloc(rs);
	if(ret3<0)goto err2;
	ret3 = mnist_model_load(rs->modelpath);
	return 0;

err2:
	mnist_label_close();
err1:
	mnist_image_close();
	return 0;
}
int mnist_exit(struct mnist_runstate* rs)
{
	mnist_label_close();
	mnist_image_close();
	return 0;
}
