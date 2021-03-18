#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "evil.h"
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
void forcedirected_3d(void*, int, void*, int, void*, int);




#define GLSL_VERSION "#version 410 core\n"
#define GLSL_PRECISION "precision mediump float;\n"
static char vs[] =
GLSL_VERSION
"layout(location = 0)in mediump vec3 v;\n"
"layout(location = 1)in mediump vec3 c;\n"
"uniform mat4 wvp;\n"
"out mediump vec3 colour;\n"
"void main(){\n"
	"colour = c;\n"
	"gl_Position = wvp * vec4(v, 1.0);\n"
"}\n";
static char fs[] =
GLSL_VERSION
"in mediump vec3 colour;\n"
"out mediump vec4 FragColor;\n"
"void main(){\n"
	"FragColor = vec4(colour, 1.0);\n"
"}\n";
static GLuint shader = 0;
static float wvp[4][4];
//data-input
struct pernode
{
        u64 type;
        u64 addr;
        u8 str[16];
};
struct perwire
{
        u16 src;
        u16 dst;
};
//data-tmp
struct vertex{
float x;
float y;
float z;
float r;
float g;
float b;
};
static struct vertex* tbuf;
static int tcnt;	//how many point
static struct vertex* vbuf;
static int vcnt;	//how many point
static u16* ibuf;
static int icnt;	//how many wire
//data-gpu
static GLuint vao = 0;
static GLuint vbo = 0;
static GLuint ibo = 0;




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






GLuint compileShader(GLenum type, const char* source)
{
	GLuint shader = glCreateShader(type);
	if(!shader)return 0;

	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);

	GLint compileStatus;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	if (GL_TRUE == compileStatus)return shader;

	GLint infoLogLength = 0;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
	if(infoLogLength){
		char* infoLog = (char*)malloc(infoLogLength);
		if (infoLog){
			glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog);
			printf("Could not compile shader %d:\n%s", type, infoLog);
			free(infoLog);
		}
		glDeleteShader(shader);
	}
	return 0;
}
GLuint compileprogram(void* v, void* f)
{
	GLuint vShader = compileShader(GL_VERTEX_SHADER, v);
	if(!vShader){
		printf("fail@compileShader: %s\n", v);
		return 0;
	}

	GLuint fShader = compileShader(GL_FRAGMENT_SHADER, f);
	if(!fShader){
		printf("fail@compileShader: %s\n", f);
		return 0;
	}

	GLuint prog = glCreateProgram();
	if(0 == prog){
		printf("ERROR : create program failed");
		exit(1);
	}

	glAttachShader(prog, vShader);
	glAttachShader(prog, fShader);
	glLinkProgram(prog);
	glDetachShader(prog, vShader);
	glDetachShader(prog, fShader);
	glDeleteShader(vShader);
	glDeleteShader(fShader);

	GLint linkStatus;
	glGetProgramiv(prog, GL_LINK_STATUS, &linkStatus);
	if(GL_TRUE == linkStatus)return prog;

	printf("ERROR : link shader program failed");
	GLint logLen;
	glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &logLen);
	if(logLen > 0)
	{
		char *log = (char*)malloc(logLen);
		GLsizei written;
		glGetProgramInfoLog(prog, logLen, &written, log);
		printf("Program log :%s\n", log);
	}

	glDeleteProgram(prog);
	return 0;
}




void glfw_drawtest(GLFWwindow* fw)
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
void glfw_drawsome(GLFWwindow* fw)
{
	int x0 = 0;
	int y0 = 0;
	int fbw;
	int fbh;
	glfwGetFramebufferSize(fw, &fbw, &fbh);

	glViewport(x0, y0, fbw, fbh);
	glScissor(x0, y0, fbw, fbh);
	glClearColor(0.1, 0.1, 0.1, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_SCISSOR_TEST);
	glEnable(GL_DEPTH_TEST);

	//shader
	glUseProgram(shader);

	//arg
	GLuint at = glGetUniformLocation(shader, "wvp");
	if(at >= 0)glUniformMatrix4fv(at, 1, GL_FALSE, (void*)wvp);

	//vao,vbo,ibo
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(struct vertex)*vcnt, vbuf);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	glDrawElements(GL_LINES, 4*icnt, GL_UNSIGNED_SHORT, (void*)0);
}



void glfw_initdata(struct pernode* cb, int cl, struct perwire* wb, int wl)
{
	int vlen = cl * sizeof(struct vertex);
	int ilen = wl * 4;
	tcnt = cl;
	tbuf = malloc(vlen);
	vcnt = cl;
	vbuf = malloc(vlen);
	icnt = wl;
	ibuf = (void*)wb;

	int j;
	for(j=0;j<cl;j++){
		vbuf[j].x = (rand()%20000)/10000.0 - 1.0;
		vbuf[j].y = (rand()%20000)/10000.0 - 1.0;
		vbuf[j].z = 0.5;
		switch(cb[j].type){
		case _hash_:
			vbuf[j].r = 1.0;
			vbuf[j].g = 0.0;
			vbuf[j].b = 0.0;
			break;
		case _file_:
			vbuf[j].r = 0.0;
			vbuf[j].g = 1.0;
			vbuf[j].b = 0.0;
			break;
		case _func_:
			vbuf[j].r = 0.0;
			vbuf[j].g = 0.0;
			vbuf[j].b = 1.0;
			break;
		default:
			vbuf[j].r = 1.0;
			vbuf[j].g = 1.0;
			vbuf[j].b = 1.0;
		}
	}

	shader = compileprogram(vs, fs);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vlen, vbuf, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, (void*)12);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ilen, ibuf, GL_STATIC_DRAW);
}
void glfw_freedata()
{
}




GLFWwindow* glfw_initwindow(int x, int y)
{
	//1.glfw
	GLFWwindow* fw = glfwCreateWindow(x, y, "42", NULL, NULL);
	if(0 == fw)
	{
		printf("error@glfwCreateWindow\n");
		return 0;
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
		return 0;
	}

	return fw;
}
void glfw_freewindow(GLFWwindow* fw)
{
	//close window
	glfwDestroyWindow(fw);

	//clear all events about this window, to ensure window closed
	int j;
	for(j=0;j<100000;j++)glfwPollEvents();
}




void render_data(struct pernode* cb, int cl, struct perwire* wb, int wl)
{
	printf("nodecnt=%d,wirecnt=%d\n", cl, wl);

	GLFWwindow* fw = glfw_initwindow(1024, 768);
	if(0 == fw)return;

	glfw_initdata(cb,cl, wb,wl);

	while(1){
		glfwMakeContextCurrent(fw);

		wvp[0][0] = 1.0;wvp[0][1] = 0.0;wvp[0][2] = 0.0;wvp[0][3] = 0.0;
		wvp[1][0] = 0.0;wvp[1][1] = 1.0;wvp[1][2] = 0.0;wvp[1][3] = 0.0;
		wvp[2][0] = 0.0;wvp[2][1] = 0.0;wvp[2][2] = 1.0;wvp[2][3] = 0.0;
		wvp[3][0] = 0.0;wvp[3][1] = 0.0;wvp[3][2] = 0.0;wvp[3][3] = 1.0;

		forcedirected_3d(tbuf,tcnt, vbuf,vcnt, ibuf,icnt);
		vbuf[0].x = vbuf[0].y = vbuf[0].z = 0.0;

		glfw_drawsome(fw);

		glfwSwapBuffers(fw);

		glfwPollEvents();
		if(glfwWindowShouldClose(fw))break;
	}

	glfw_freedata();

	glfw_freewindow(fw);
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
