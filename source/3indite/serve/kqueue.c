#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define MAXSIZE 4096
int servesocket(void*, int, void*, int);




int startsocket(int port)
{
	//create
	tcpfd = socket(AF_INET, SOCK_STREAM, 0);
	if (tcpfd == -1)
	{
		printf("error@socket\n");
		return 0;
	}
	if(tcpfd > MAXSIZE)
	{
		printf("tcpfd>4096\n");
		return 0;
	}

	//reuse
	ret = 1;
	ret = setsockopt(tcpfd, SOL_SOCKET, SO_REUSEADDR, &ret, 4);
	if(ret<0)
	{
		printf("error@setsockopet\n");
		return 0;
	}

	//self
	memset(&self, 0, sizeof(struct sockaddr_in));
	self.sin_family = AF_INET;
	self.sin_port = htons(port);
	self.sin_addr.s_addr = htons(INADDR_ANY);
	ret = bind(tcpfd, (void*)&self, sizeof(struct sockaddr_in));
	if(ret == -1)
	{
		printf("error@bind\n");
		close(tcpfd);
		return 0;
	}

	//kqueue

	//listen
	listen(tcpfd, 5);
}
