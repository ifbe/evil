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
	WSABUF bufing;
	int count;
	int stage;
	SOCKET fd;
};
struct object
{
	u8 self[0x40];
	struct per_io_data data[1];
};
static struct object obj[0x1000];
static u8 wbuf[0x100000];
static HANDLE iocpfd;
static SOCKET listenfd;




void iocp_add(SOCKET fd)
{
	u32* pfd = (void*)(obj[fd/4].self);
	struct per_io_data* pio = (void*)(obj[fd/4].data);

	*pfd = fd;
	CreateIoCompletionPort(
		(void*)fd,
		iocpfd,
		(ULONG_PTR)pfd,
		0
	);

	pio->stage = 1;
	pio->bufing.buf = malloc(4096);
	pio->bufing.len = 4096;
}
void iocp_del(SOCKET fd)
{
	struct per_io_data* pio = (void*)(obj[fd/4].data);
	if(pio->bufing.buf)free(pio->bufing.buf);
}
void iocp_mod(SOCKET fd)
{
	int ret;
	DWORD tran = 0;
	DWORD flag = 0;
	struct per_io_data* pio = (void*)(obj[fd/4].data);
	ret = WSARecv(fd, &(pio->bufing), 1, &tran, &flag, (void*)pio, NULL);
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
	//printf("[%x]close\n",fd);
	return 0;
}
int myread(SOCKET fd)
{
	int len = obj[fd/4].data[0].count;
	u8* buf = obj[fd/4].data[0].bufing.buf;

	int ret = servesocket(wbuf, 0x100000, buf, len);
printf("haha=%d\n",ret);
	if(ret <= 0)return 0;

	ret = writesocket(fd, wbuf, ret);
	return ret;
}
void* WINAPI iocpthread(void* arg)
{
	u32* pfd = 0;
	struct per_io_data* pio = NULL;
	int th;
	int ret;
	SOCKET fd;
	SOCKET cc;
	DWORD tran = 0;
	DWORD flag = 0;

	th = GetCurrentThreadId();
	//printf("%d\n",th);

	while(1)
	{
		ret = GetQueuedCompletionStatus(
			iocpfd,
			&tran,
			(void*)&pfd,
			(void*)&pio,
			INFINITE
		);
		if(ret == 0)continue;

		fd = *pfd;
		cc = pio->fd;
		//printf("th=%d,tran=%d,listen=%d,this=%d\n", th, tran, fd, cc);

		//accept
		if(pio->stage == 0)
		{
			printf("[%x,%x]++++\n", fd, cc);

			iocp_add(cc);
			iocp_mod(cc);
		}
		else if(tran == 0)
		{
			printf("[%x]----\n",fd);
			stopsocket(fd);
			continue;
		}
		else
		{
			printf("[%x]####\n",fd);
			myread(fd);
			stopsocket(fd);
		}
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
	iocp_add(listenfd);

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
	u32* pfd;
	struct per_io_data* pio;
	for(j=0;j<0x400;j++)
	{
		tmp = WSASocket(
			AF_INET, SOCK_STREAM, IPPROTO_TCP,
			0, 0, WSA_FLAG_OVERLAPPED
		);
		if(tmp&0x3)printf("%d\n", tmp);

		pfd = (void*)(obj[tmp/4].self);
		*pfd = tmp;

		pio = (void*)(obj[tmp/4].data);
		pio->count = 0;
		pio->stage = 0;
		pio->fd = tmp;

		ret = acceptex(
			listenfd, tmp,
			pfd, 0, 0x20, 0x20, 0,
			(void*)pio
		);
	}

	while(1)Sleep(1000000);
	return listenfd;
}