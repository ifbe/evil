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

	j=0;
	k=0;
	while(1)
	{
		val = buf[j];
		if( ( (val >= '0')&&(val <= '9') ) |
			( (val >= 'A')&&(val <= 'Z') ) |
			( (val >= 'a')&&(val <= 'z') ) |
			(val == '.') ){flag = 1;}
		else{flag = 0;}

		if((flag == 1)&&(k == 0))k = j;
		if(k != 0)
		{
			if(j+1 == len)
			{
				flag = 0;
				j++;
			}
			if(flag == 0)
			{
				worker_write(buf+k, j-k, 3, 0);
				k = 0;
			}
		}

		j++;
		if(j >= len)break;
	}
}