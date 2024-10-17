
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
void chipdata_start(int);
void chipdata_stop();
void chipindex_start(int);
void chipindex_stop();
void filedata_start(int);
void filedata_stop();
void filemd5_start(int);
void filemd5_stop();
void funcdata_start(int);
void funcdata_stop();
void funcindex_start(int);
void funcindex_stop();
void pindata_start(int);
void pindata_stop();
void pinindex_start(int);
void pinindex_stop();
void pointdata_start(int);
void pointdata_stop();
void pointindex_start(int);
void pointindex_stop();
void shapedata_start(int);
void shapedata_stop();
void shapeindex_start(int);
void shapeindex_stop();
void strdata_start(int);
void strdata_stop();
void strhash_start(int);
void strhash_stop();
void relation_start(int);
void relation_stop();
//
void learn_init(){
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


	filemd5_create();
	filedata_create();

	funcindex_create();
	funcdata_create();

	strhash_create();
	strdata_create();

	relation_create();
}
void learn_exit(){
	relation_delete();

	strhash_delete();
	strdata_delete();

	funcindex_delete();
	funcdata_delete();

	filemd5_delete();
	filedata_delete();
}




void readthemall(int j)
{
	learn_init();

	//chipdata_start(j);
	chipindex_start(j);

	filedata_start(j);
	filemd5_start(j);

	funcdata_start(j);
	funcindex_start(j);

	//pindata_start(j);
	pinindex_start(j);

	pointdata_start(j);
	pointindex_start(j);

	//shapedata_start(j);
	shapeindex_start(j);

	strdata_start(j);
	strhash_start(j);

	relation_start(j);
}
void writethemall(int unused)
{
	relation_stop();

	//chipdata_stop();
	chipindex_stop();

	filedata_stop();
	filemd5_stop();

	funcdata_stop();
	funcindex_stop();

	//pindata_stop();
	pinindex_stop();

	pointdata_stop();
	pointindex_stop();

	//shapedata_stop();
	shapeindex_stop();

	strdata_stop();
	strhash_stop();

	learn_exit();
}
