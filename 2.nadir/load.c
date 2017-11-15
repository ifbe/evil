void chipdata_start(int);
void chipindex_start(int);
void filedata_start(int);
void filemd5_start(int);
void funcdata_start(int);
void funcindex_start(int);
void pindata_start(int);
void pinindex_start(int);
void pointdata_start(int);
void pointindex_start(int);
void shapedata_start(int);
void shapeindex_start(int);
void strdata_start(int);
void strhash_start(int);
void connect_start(int);




void readall(int j)
{
	//chipdata_start(j);
	chipindex_start(j);

	filedata_start(j);
	filemd5_start(j);

	funcdata_start(j);
	funcindex_start(j);

	//pindata_start(j);
	pinindex_start(j);

	pointindex_start(j);
	shapeindex_start(j);

	strdata_start(j);
	strhash_start(j);

	connect_start(j);
}
void writeall()
{
}