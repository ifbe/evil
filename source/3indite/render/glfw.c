#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>




static void callback_keyboard(GLFWwindow* fw, int key, int scan, int action, int mods)
{
/*	struct event e;
	struct supply* ogl = glfwGetWindowUserPointer(fw);
	//printf("key=%x,scan=%x,action=%x,mods=%x\n", key, scan, action, mods);

	if(0 == action)return;
	if(0x118 == key)return;		//capslock
	if((key >= 0x154)&&(key <= 0x15b))return;	//ctrl,alt...

	if(0x100 == key)
	{
		e.why = 0x1b;
		e.what = _kbd_;
	}
	else if((key >= 0x106)&&(key <= 0x109))
	{
		if(key == 0x109)e.why = 0x48;		//up
		else if(key == 0x108)e.why = 0x50;	//down
		else if(key == 0x107)e.why = 0x4b;	//left
		else if(key == 0x106)e.why = 0x4d;	//right
		e.what = _kbd_;
	}
	else if((key >= 0x122)&&(key <= 0x12d))
	{
		e.why = key + 0xf1 - 0x122;
		e.what = _kbd_;
	}
	else if(0x101 == key)
	{
		e.why = 0xd;
		e.what = _char_;
	}
	else if(0x102 == key)
	{
		e.why = '\t';
		e.what = _char_;
	}
	else if(0x103 == key)
	{
		e.why = 0x8;
		e.what = _char_;
	}
	else if((key >= 'A')&&(key <= 'Z'))
	{
		if(0 == (mods&1))key += 0x20;
		e.why = key;
		e.what = _char_;
	}
	else
	{
		if(mods&1)key = uppercase[key-0x20];
		e.why = key;
		e.what = _char_;
	}

	e.where = (u64)ogl;
	restorestackdeliverevent(ogl, &e);*/
}
static void callback_scroll(GLFWwindow* fw, double x, double y)
{
/*	struct supply* ogl = glfwGetWindowUserPointer(fw);
	//printf("%llx: %f,%f\n", (u64)ogl, x, y);

	double xpos, ypos;
	glfwGetCursorPos(fw, &xpos, &ypos);

	struct event ev;
	ev.where = (u64)ogl;
	ev.what = 0x4070;

	short* t = (void*)&ev.why;
	t[0] = (short)xpos;
	t[1] = (short)ypos;
	t[2] = (short)y;
	t[3] = (y>0.0) ? 'f' : 'b';
	restorestackdeliverevent(ogl, &ev);*/
}
static void callback_mouse(GLFWwindow* fw, int button, int action, int mods)
{
/*	double xpos, ypos;
	glfwGetCursorPos(fw, &xpos, &ypos);

	struct event ev;
	ev.what = (1 == action) ? 0x2b70 : 0x2d70;

	t[0] = (short)xpos;
	t[1] = (short)ypos;
	t[3] = (0 == button) ? 'l' : 'r';

	restorestackdeliverevent(ogl, &ev);*/
}
static void callback_move(GLFWwindow* fw, double xpos, double ypos)
{
/*	int btn;
	if(GLFW_PRESS == glfwGetMouseButton(fw, GLFW_MOUSE_BUTTON_LEFT))btn = 1;
	else btn = 0;

	restorestackdeliverevent(ogl, &ev);*/
}
static void callback_drop(GLFWwindow* fw, int count, const char** paths)
{
	char dragdata[0x1000];
	int j,ret=0;

	for(j=0;j<count;j++)
	{
		ret += snprintf(dragdata+ret, 0x1000-ret, "%s\n", paths[j]);
		printf("%s\n", paths[j]);
	}
}
static void callback_reshape(GLFWwindow* fw, int w, int h)
{
/*	struct supply* ogl = glfwGetWindowUserPointer(fw);
	ogl->fbwidth = w;
	ogl->fbheight = h;

	glfwGetWindowSize(fw, &w, &h);
	ogl->width = w;
	ogl->height = h;*/
}
void glfw_drawsome(GLFWwindow* fw)
{
	int w,h;
	glfwGetWindowSize(fw, &w, &h);
	int fbw,fbh;
	glfwGetFramebufferSize(fw, &fbw, &fbh);

	float x = 256;
	float y = h-1 - 256;
	float r = x / w;
	float g = y / h;
	float b = 0.0;
	float a = 0.0;

	//fbw != w, fbh != h, so ...
	x *= fbw / w;
	y *= fbh / h;

	//viewport can not change clearcolor area
	glViewport(0, 0, x, y);

	//but scissor can ...
	glScissor(0, 0, x, y);
	glEnable(GL_SCISSOR_TEST);

	//clear screen
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT);	//GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT
}
void glfw_openwindow(int x, int y)
{
	//1.glfw
	GLFWwindow* fw = glfwCreateWindow(x, y, "42", NULL, NULL);
	if(0 == fw)
	{
		printf("error@glfwCreateWindow\n");
		return;
	}

	//2.setup
	int fbx,fby;
	glfwGetFramebufferSize(fw, &fbx, &fby);
	printf("fbx=%d,fby=%d\n",fbx,fby);

	//3.callback
	glfwSetDropCallback(fw, callback_drop);
	glfwSetKeyCallback(fw, callback_keyboard);
	glfwSetScrollCallback(fw, callback_scroll);
	glfwSetCursorPosCallback(fw, callback_move);
	glfwSetMouseButtonCallback(fw, callback_mouse);
	glfwSetFramebufferSizeCallback(fw, callback_reshape);

	//2.glew
	glfwMakeContextCurrent(fw);
	glewExperimental = 1;
	if(glewInit() != GLEW_OK)
	{
		printf("error@glewInit\n");
		return;
	}


	while(1){
		glfwMakeContextCurrent(fw);

		glfw_drawsome(fw);

		glfwSwapBuffers(fw);

		if(glfwWindowShouldClose(fw))break;
		glfwPollEvents();
	}

	glfwDestroyWindow(fw);
}
void render_data(void* cb, int cl, void* lb, int ll)
{
	printf("nodecnt=%d,wirecnt=%d\n", cl, ll);

	glfw_openwindow(1024, 768);
}




void render_free()
{
	printf("@render_free\n");

	glfwTerminate();
}
void render_init(void* cb, int cl, void* lb, int ll)
{
	printf("@render_init\n");

	if(glfwInit() == 0)
	{
		printf("error@glfwInit\n");
		return;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}
