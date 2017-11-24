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
void connect_start(int);
void connect_stop();




void readthemall(int j)
{
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

	connect_start(j);
}
void writethemall()
{
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

	connect_stop();
}