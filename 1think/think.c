//
void strdata_start(int);
void strdata_stop();
void strhash_start(int);
void strhash_stop();
void relation_start(int);
void relation_stop();
//
void think_substr();




void think(int argc, char** argv)
{
	strdata_start(1);
	strhash_start(1);
	relation_start(1);

	think_substr();

	strdata_stop();
	strhash_stop();
	relation_stop();
}
