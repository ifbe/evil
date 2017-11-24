//
void strdata_start(int);
void strdata_stop();
void strhash_start(int);
void strhash_stop();
void connect_start(int);
void connect_stop();
//
void think_substr();
void think_mergechip();




void think(int argc, char** argv)
{
	strdata_start(1);
	strhash_start(1);
	connect_start(1);

	think_substr();
	think_mergechip();

	strdata_stop();
	strhash_stop();
	connect_stop();
}