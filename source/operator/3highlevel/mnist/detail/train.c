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
	fseek(imagefile, 16+id*28*28, SEEK_SET);
	fread(imagemem, 28*28, 1, imagefile);
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


int mnist_train_one(int id)
{
	u8* image = mnist_image_get(id);
	printf("%s: image=%p\n", __func__, image);
	u8* label = mnist_label_get(id);
	printf("%s: label=%p\n", __func__, label);

	int x,y;
	printf("label=%d\n", label[0]);
	for(y=0;y<28;y++){
		for(x=0;x<28;x++){
			printf("%d%c", image[y*28+x]/26, (x+1==28) ? '\n' : ' ');
		}
	}

	return 0;
}


int mnist_train(int argc, char** argv)
{
	char* imagepath = argv[1];
	printf("imagepath=%s\n", imagepath);
	char* labelpath = argv[2];
	printf("label=%s\n", labelpath);
	if(!imagepath)return 0;
	if(!labelpath)return 0;

	int ret1 = mnist_image_open(imagepath);
	if(ret1<0)goto err1;
	printf("image: file=%p, mem=%p\n", imagefile, imagemem);

	int ret2 = mnist_label_open(labelpath);
	if(ret2<0)goto err2;
	printf("label: file=%p, mem=%p\n", labelfile, labelmem);

	int id = 26;
	int ret3;
	while(1){
		ret3 = scanf("%d", &id);
		if(ret3 <= 0)break;

		mnist_train_one(id);
	}

err2:
	mnist_label_close();
err1:
	mnist_image_close();

	return 0;
}