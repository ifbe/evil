#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ws2tcpip.h>	//IP_HDRINCL
#include<ws2bth.h>		//BTHPROTO_RFCOMM
#include<winsock2.h>
#include<mswsock.h>
#include<windows.h>
#include<pthread.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
int servesocket(char* wbuf, int wlen, char* buf, int len);




struct per_io_data
{
	OVERLAPPED overlap;
	SOCKET fd;
	int stage;
	WSABUF bufing;
	WSABUF bufdone;
};
struct object
{
	u64 type_sock;
	u8 self[0x40];
	struct per_io_data data[1];
};
static struct object obj[0x1000];
static u8 wbuf[0x100000];
static HANDLE iocpfd;
static SOCKET listenfd;




void stopwatcher()
{
}
void startwatcher(SOCKET fd)
{
	u32* p = (void*)(obj[fd/4].self);
	*p = fd;
	CreateIoCompletionPort(
		(void*)fd,
		iocpfd,
		(ULONG_PTR)p,
		0
	);
}




int readsocket(SOCKET fd, u8* buf, int len)
{
	int c,j;
	char* p;
	struct per_io_data* pov;

	pov = (void*)(obj[fd/4].data);
	p = pov->bufdone.buf;
	c = pov->bufdone.len;
	for(j=0;j<c;j++)buf[j] = p[j];
//printf("(read)len=%d\n",c);
	free(pov->bufdone.buf);

	if(c == 0)return -1;	//disconnect
	return c;
}
int writesocket(SOCKET fd, u8* buf, int len)
{
	int ret;
	DWORD dwret;
	WSABUF wb;

	wb.buf = buf;
	wb.len = len;
	ret = WSASend(fd, &wb, 1, &dwret, 0, 0, 0);

	//printf("@send:len=%d,ret=%d,err=%d\n",len,ret,GetLastError());
	return len;
}
int myaccept()
{
}
int stopsocket(SOCKET fd)
{
	LPFN_DISCONNECTEX disconnectex = NULL;
	GUID guiddisconnectex = WSAID_DISCONNECTEX;
	DWORD dwret = 0;
	int ret = WSAIoctl(
		listenfd,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guiddisconnectex,
		sizeof(guiddisconnectex),
		&disconnectex,
		sizeof(disconnectex),
		&dwret,
		NULL,
		NULL
	);
	if(ret != 0)
	{
		printf("error@WSAIoctl\n");
		return 0;
	}

	disconnectex(fd, 0, TF_REUSE_SOCKET, 0);
	printf("[%x]close\n",fd);
	return 0;
}
int myread(SOCKET fd)
{
	WSABUF* rb = &obj[fd/4].data[0].bufdone;
	int ret = servesocket(wbuf, 0x100000, rb->buf, rb->len);
	if(ret <= 0)return 0;

	ret = writesocket(fd, wbuf, ret);
	return ret;
}
void* WINAPI iocpthread(void* arg)
{
	struct per_io_data* pov = NULL;
	u32* key = 0;
	char temp[4096];
	int th;
	int ret;

	HANDLE hh;
	SOCKET fd;
	DWORD trans = 0;
	DWORD flag = 0;

	th = GetCurrentThreadId();
	//printf("%d\n",th);

	while(1)
	{
		ret = GetQueuedCompletionStatus(
			iocpfd,
			&trans,
			(void*)&key,
			(void*)&pov,
			INFINITE
		);
		if(ret == 0)continue;

		fd = pov->fd;
		printf("th=%d,ret=%d,trans=%d,listen=%d,this=%d\n",
		th, ret, trans, *key, fd);

		//accept
		if(pov->stage == 0)
		{
			hh = CreateIoCompletionPort(
				(void*)fd,
				iocpfd,
				(ULONG_PTR)(obj[fd/4].self),
				0
			);

			pov->stage = 1;
			obj[fd/4].type_sock = 't';

			//eventwrite('+', __fd__, fd/4, 0);
			printf("[%x]++++,hh=%llx\n",fd/4,hh);
		}
		else if(trans == 0)
		{
			printf("[%x]----\n",fd/4);
			stopsocket(fd);
			continue;
		}
		else
		{
			pov->stage = 1;
			pov->bufdone.buf = pov->bufing.buf;
			pov->bufdone.len = trans;
			//eventwrite('@', __fd__, fd/4, 0);
			printf("[%x]####\n",fd/4);
			myread(fd);
			stopsocket(fd);
		}

		//all
		pov->bufing.buf = malloc(4096);
		pov->bufing.len = 4096;
		ret = WSARecv(fd, &(pov->bufing), 1, &trans, &flag, (void*)pov, NULL);
		//printf("(recv)ret=%d,err=%d\n", ret, WSAGetLastError());
	}
	return 0;
}




int startsocket(int port)
{
	int ret;
	int addrlen = sizeof(SOCKADDR_IN);
	SOCKADDR_IN servaddr;

	u64 temp;
	iocpfd = CreateIoCompletionPort(
		INVALID_HANDLE_VALUE,
		NULL,
		0,
		4
	);

	int j;
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	for(j=0;j<info.dwNumberOfProcessors*2;j++)
	{
		pthread_create((void*)&temp, NULL, iocpthread, 0);
	}

	//socket
	WSADATA data; 
	WORD sockVersion = MAKEWORD(2,2);
	if(WSAStartup(sockVersion, &data) != 0)
	{
		printf("error@WSAStartup\n");
		return 0;
	}

	//server.1
	listenfd = WSASocket(
		AF_INET, SOCK_STREAM, IPPROTO_TCP,
		0, 0, WSA_FLAG_OVERLAPPED
	);
	if(listenfd == INVALID_SOCKET)
	{
		printf("error@wsasocket:%d\n",GetLastError());
		return 0;
	}

	//server.2
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(port);
	servaddr.sin_addr.s_addr = INADDR_ANY;

	//server.3
	ret = bind(listenfd, (SOCKADDR*)&servaddr, addrlen);
	if(ret == SOCKET_ERROR)
	{
		printf("error@bind\n");
		closesocket(listenfd);
		return 0;
	}

	//server.4
	ret = listen(listenfd, SOMAXCONN);
	if(ret == -1)
	{
		printf("error@listen\n");
		closesocket(listenfd);
		return 0;
	}

	//server.5
	startwatcher(listenfd);

	//client.1
	LPFN_ACCEPTEX acceptex = NULL;
	GUID guidacceptex = WSAID_ACCEPTEX;
	DWORD dwret = 0;
	ret = WSAIoctl(
		listenfd,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guidacceptex,
		sizeof(guidacceptex),
		&acceptex,
		sizeof(acceptex),
		&dwret,
		NULL,
		NULL
	);
	if(ret != 0)
	{
		printf("error@WSAIoctl\n");
		return 0;
	}

	//clients.2
	SOCKET tmp;
	void* pdata;
	struct per_io_data* pov;
	for(j=0;j<1000;j++)
	{
		tmp = WSASocket(
			AF_INET, SOCK_STREAM, IPPROTO_TCP,
			0, 0, WSA_FLAG_OVERLAPPED
		);
		if((tmp&0x3)|(tmp>=0x4000))printf("%d\n", tmp/4);

		//
		pdata = (void*)(obj[tmp/4].self);
		pov = (void*)(obj[tmp/4].data);
		pov->fd = tmp;
		pov->stage = 0;
		ret = acceptex(
			listenfd, tmp,
			pdata, 0, 0x20, 0x20, 0,
			(void*)pov
		);
	}

	while(1)Sleep(1000000);
	return listenfd;
}