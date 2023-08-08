#define WINVER 0x0601
#define WM_POINTERUPDATE 0x0245
#define WM_POINTERDOWN 0x0246
#define WM_POINTERUP 0x0247
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define hex64(a,b,c,d,e,f,g,h) (hex32(a,b,c,d) | (((u64)hex32(e,f,g,h))<<32))
#include<stdio.h>
#include<stdlib.h>
#include<windows.h>
#include<windowsx.h>
#include<winuser.h>
#include<commctrl.h>
#include<pthread.h>
void drawstring(void* buf, u32 rgb, int w, int h,
	int x, int y, u8* str, int len);
void drawline(void* buf, u32 rgb, int w, int h,
	int x0, int y0, int x1, int y1);
void forcedirected_2d(void*, int, void*, int, void*, int);




static HWND wnd;
static HDC dc;
static u8 buf[1024*1024*4];
static int width = 1024;
static int height = 768;
static float scale = 128.0;
//global
static HANDLE hStartEvent;
static WNDCLASS wc;
static char* AppTitle="evil";
static char dragpath[MAX_PATH];
//temp
static int leftdown=0;
static int rightdown=0;
static POINT pt, pe;
static RECT rt, re;
//
struct context
{
	u64 type;
	u64 addr;
	u8 str[16];
};
struct pair
{
	u16 parent;
	u16 child;
};
static struct context* ctxbuf = 0;
static int ctxlen = 0;
//
static struct pair* lbuf;
static int llen;
static float* vbuf;
static int vlen;
static float* obuf;




void windowwrite()
{
	BITMAPINFO info;
	int x0,y0,x1,y1,m,n,j;

	info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	info.bmiHeader.biWidth = width;
	info.bmiHeader.biHeight = -height;
	info.bmiHeader.biPlanes = 1;
	info.bmiHeader.biBitCount = 32;
	info.bmiHeader.biCompression = 0;
	info.bmiHeader.biSizeImage = width*height*4;
	info.bmiHeader.biXPelsPerMeter = 0;
	info.bmiHeader.biYPelsPerMeter = 0;
	info.bmiHeader.biClrUsed = 0;
	info.bmiHeader.biClrImportant = 0;
	info.bmiColors[0].rgbBlue = 255;
	info.bmiColors[0].rgbGreen = 255;
	info.bmiColors[0].rgbRed = 255;
	info.bmiColors[0].rgbReserved = 255;

	for(j=0;j<4;j++)
	{
		forcedirected_2d(
			obuf, vlen,
			vbuf, vlen,
			lbuf, llen
		);
		vbuf[0] = vbuf[1] = vbuf[2] = 0.0;
		vbuf[3] = vbuf[4] = vbuf[5] = 0.0;
	}

	for(j=0;j<width*height*4;j++)buf[j] = 0;
	for(j=0;j<llen;j++)
	{
		m = 3*(lbuf[j].parent&0xfffe);
		n = 3*(lbuf[j].child&0xfffe);
		x0 = 512 + (int)(scale * vbuf[m+0]);
		y0 = 384 + (int)(scale * vbuf[m+1]);
		x1 = 512 + (int)(scale * vbuf[n+0]);
		y1 = 384 + (int)(scale * vbuf[n+1]);
		drawline(buf, 0xffffff, width, height, x0,y0,x1,y1);
	}
	for(j=ctxlen-1;j>=0;j--)
	{
		x0 = 512 + (int)(scale * vbuf[6*j+0]);
		y0 = 384 + (int)(scale * vbuf[6*j+1]);
		if(ctxbuf[j].type == hex32('h','a','s','h'))
		{
			drawstring(
				buf, 0xff0000, width, height,
				x0, y0, ctxbuf[j].str, 16
			);
		}
	}

	//write bmp to win
	SetDIBitsToDevice(
		dc,
		0, 0,				//目标位置x,y
		width, height,		//dib宽,高
		0, 0,				//来源起始x,y
		0, height,			//起始扫描线,数组中扫描线数量,
		buf,				//rbg颜色数组
		&info,				//bitmapinfo
		DIB_RGB_COLORS		//颜色格式
	);
	//printf("result:%x\n",result);
}
void windowread()
{
}




LRESULT CALLBACK WindowProc(HWND wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
		case WM_MOUSEWHEEL:
		{
			u64 x,y,k;
			GetCursorPos(&pt);
			ScreenToClient(wnd, &pt);

			if( ((wparam>>16) & 0xffff ) < 0xf000 )
			{
				scale*=2;
				k = 'f';
			}
			else
			{
				scale/=2;
				k = 'b';
			}
			x = ((lparam&0xffff)<<16);
			y = (lparam&0xffff0000);
			printf("%c@:%d,%d\n",k,x,y);

			return 0;
		}

		case WM_MOUSEMOVE:
		{
			u64 x,y,k;
			if((leftdown>0)&&(rightdown>0))
			{
				GetCursorPos(&pe);		// 获取光标指针的新位置
				re.left=rt.left+(pe.x - pt.x);		// 窗口新的水平位置
				re.top =rt.top+(pe.y - pt.y);		// 窗口新的垂直位置
				MoveWindow(wnd, re.left, re.top, re.right, re.bottom, 1);// 移动窗口
				GetCursorPos(&pt);			// 获取鼠标当前位置
				GetWindowRect(wnd, &rt);	// 获取窗口位置与大小
				return 0;
			}
			else if(rightdown>0)k = 'r';
			else if(leftdown>0)k = 'l';
			else k = '?';

			x = ((lparam&0xffff)<<16);
			y = (lparam&0xffff0000);
			printf("%c@:%d,%d\n",k,x,y);
			return 0;
		}

		case WM_LBUTTONUP:
		{
			u64 x,y,k;
			leftdown=0;

			x = ((lparam&0xffff)<<16);
			y = (lparam&0xffff0000);
			printf("l-:%d,%d\n",x,y);
			return 0;
		}

		case WM_RBUTTONUP:
		{
			u64 x,y,k;
			rightdown=0;

			x = ((lparam&0xffff)<<16);
			y = (lparam&0xffff0000);
			printf("r-:%d,%d\n",x,y);
			return 0;
		}

		case WM_LBUTTONDOWN:
		{
			u64 x,y,k;
			leftdown=1;
			GetCursorPos(&pt);		// 获取鼠标光标指针当前位置

			if(rightdown>0)
			{
				GetWindowRect(wnd, &rt);	// 获取窗口位置与大小
				re.right=rt.right-rt.left;	// 保存窗口宽度
				re.bottom=rt.bottom-rt.top;	// 保存窗口高度
			}

			x = ((lparam&0xffff)<<16);
			y = (lparam&0xffff0000);
			printf("l+:%d,%d\n",x,y);
			return 0;
		}

		case WM_RBUTTONDOWN:
		{
			u64 x,y,k;
			rightdown=1;
			GetCursorPos(&pt);

			if(leftdown>0)
			{
				GetWindowRect(wnd, &rt);
				re.right=rt.right-rt.left;
				re.bottom=rt.bottom-rt.top;
			}

			x = ((lparam&0xffff)<<16);
			y = (lparam&0xffff0000);
			printf("r+:%d,%d\n",x,y);
			return 0;
		}

		case WM_DROPFILES:
		{
			HDROP hDrop = (HDROP)wparam;
			UINT nFileNum = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0); // 拖拽文件个数
			int i;
			for (i = 0; i < nFileNum; i++)  
			{
				DragQueryFile(hDrop, i, dragpath, MAX_PATH);//获得拖曳的文件名
			}
			DragFinish(hDrop);      //释放hDrop

			printf("dropfile\n");
			return 0;
		}

		case WM_SIZE:
		{
			width = lparam&0xffff;
			height = (lparam>>16)&0xffff;
			printf("wm_size:%d,%d\n", width, height);
			return 0;
		}

		case WM_PAINT:
		{
			printf("WM_PAINT\n");
			goto theend;
		}

		case WM_CLOSE:
		{
			PostQuitMessage(0);
			return 0;
		}

		case WM_DESTROY:
		{
			DestroyWindow(wnd);
			return 0;
		}
	}

theend:
	return DefWindowProc(wnd, msg, wparam, lparam);
}




void createmywindow()
{
	//创建窗口
	wnd = CreateWindow(
		AppTitle, AppTitle, WS_OVERLAPPEDWINDOW,		//WS_POPUP | WS_MINIMIZEBOX=无边框
		100, 100, width+16, height+39,
		NULL, NULL, 0, NULL);
	if(!wnd)return;

	//dc
	dc = GetDC(wnd);

	//透明
	LONG t = GetWindowLong(wnd, GWL_EXSTYLE);
	SetWindowLong(wnd, GWL_EXSTYLE, t | WS_EX_LAYERED);
	SetLayeredWindowAttributes(wnd, 0, 0xf8, LWA_ALPHA);

	//显示窗口
	ShowWindow(wnd, SW_SHOW);
	UpdateWindow(wnd);

	//打开触摸
	RegisterTouchWindow(wnd, 0);

	//打开拖拽
	typedef BOOL (WINAPI *ChangeWindowMessageFilterProc)(UINT, u32);
	DragAcceptFiles(wnd, TRUE);

	HMODULE hUser = LoadLibraryA("user32.dll");
	if(!hUser){printf("failed to load\n");exit(-1);}

	ChangeWindowMessageFilterProc hProc;
	hProc = (ChangeWindowMessageFilterProc)GetProcAddress(hUser, "ChangeWindowMessageFilter");
	if(!hProc){printf("can't drag\n");exit(-1);}

	hProc(WM_COPYDATA, 1);
	hProc(WM_DROPFILES, 1);
	hProc(0x0049, 1);
}
void* render_thread(void* arg)
{
	MSG msg;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = 0;				//hInst;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOWFRAME;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = AppTitle;
	if(!RegisterClass(&wc))
	{
		printf("error@RegisterClass\n");
		return 0;
	}

	//create queue
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	//fire event
	SetEvent(hStartEvent);

	//open window
	createmywindow((void*)(msg.lParam));

	//wait
	while(GetMessage(&msg, NULL, 0, 0))
	{
		windowwrite();
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
void render_data(void* cb, int cl, void* lb, int ll)
{
	int j;
	float r,g,b;
	float* vv;
	void* buf;

	ctxbuf = cb;
	ctxlen = cl;
	llen = ll;
	vlen = cl*2;

	buf = (void*)vbuf;
	for(j=0;j<ctxlen;j++)
	{
		vv = buf + 24*j;

		vv[0] = (rand()&0xffff) / 65536.0;
		vv[1] = (rand()&0xffff) / 65536.0;
		vv[2] = (rand()&0xffff) / 65536.0;

		vv[3] = 0.0;
		vv[4] = 0.0;
		vv[5] = 0.0;
	}
}




void render_init(void* cb, int cl, void* lb, int ll)
{
	u64 id;
	ctxbuf = cb;
	ctxlen = cl;

	lbuf = lb;
	vbuf = malloc(0x200000);
	obuf = malloc(0x200000);

	//createevent
	hStartEvent = CreateEvent(0,FALSE,FALSE,0);

	//uievent
	pthread_create((void*)&id, NULL, render_thread, 0);

	//waitevent
	WaitForSingleObject(hStartEvent,INFINITE);

	//deleteevent
	CloseHandle(hStartEvent);
}
void render_free()
{
}
