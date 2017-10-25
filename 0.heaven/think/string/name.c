int worker_write(char* buf, int len, int type, int haha);




void name_write(char* buf, int len)
{
	int j,k,val,flag;
	if(buf == 0)return;
	if(len == 0)
	{
		while(buf[len] != 0)len++;
	}
	worker_write(buf, len, 4, 0);

	j = 0;
	k = -1;
	while(1)
	{
		if(j >= len)val = 0;
		else val = buf[j];

		if( ( (val >= '0')&&(val <= '9') ) |
			( (val >= 'A')&&(val <= 'Z') ) |
			( (val >= 'a')&&(val <= 'z') ) |
			(val == '.'))
		{
			if(k<0)k = j;
			flag = 0;
		}
		else flag = 1;

		if( (flag == 1) && (j-k < len) )
		{
			worker_write(buf+k, j-k, 3, 0);
			//printf("%.*s\n", j-k, buf+k);
			k = -1;
		}

		j++;
		if(j > len)break;
	}
}