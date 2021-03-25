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
typedef float vec1[1];
typedef float vec2[2];
typedef float vec3[3];
typedef float vec4[4];
typedef float mat1[1][1];
typedef float mat2[2][2];
typedef float mat3[3][3];
typedef float mat4[4][4];
void forcedirected_3d(void*, int, void*, int, void*, int);




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
//
struct camera{
	vec4 vl;	//[00,0f]: left
	vec4 vr;	//[10,1f]: right
	vec4 vn;	//[20,2f]: near
	vec4 vf;	//[30,3f]: far
	vec4 vb;	//[40,4f]: bottom
	vec4 vt;	//[50,5f]: upper
	vec4 vq;	//[60,6f]: info
	vec4 vc;	//[70,7f]: center
};




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
//
static struct camera cam;
static float wvp[4][4];
//wire
static struct vertex* tbuf;
static int tcnt;	//how many point
static struct vertex* vbuf;
static int vcnt;	//how many point
static u16* ibuf;
static int icnt;	//how many wire
static GLuint vao = 0;
static GLuint vbo = 0;
static GLuint ibo = 0;
//node
static struct vertex* node_vbuf;
static GLuint nodevao = 0;
static GLuint nodevbo = 0;
static GLuint nodeibo = 0;
//str
static struct vertex* str_vbuf;
static GLuint strvao = 0;
static GLuint strvbo = 0;
static GLuint stribo = 0;




static void callback_keyboard(GLFWwindow* fw, int key, int scan, int action, int mods)
{
	printf("key=%x,scan=%x,action=%x,mods=%x\n", key, scan, action, mods);
	switch(key){
	case 0x109://up
		cam.vc[1] += 100.0;
		break;
	case 0x108://down
		cam.vc[1] -= 100.0;
		break;
	case 0x107://left
		break;
	case 0x106://right
		break;
	}

/*	struct event e;
	struct supply* ogl = glfwGetWindowUserPointer(fw);

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






void world2view_rh2lh(mat4 m, struct camera* s)
{
	float x,y,z;
	float cx = s->vc[0];
	float cy = s->vc[1];
	float cz = s->vc[2];

	x = s->vr[0];
	y = s->vr[1];
	z = s->vr[2];
	m[0][0] = x;
	m[0][1] = y;
	m[0][2] = z;
	m[0][3] = - cx*x - cy*y - cz*z;

	x = s->vt[0];
	y = s->vt[1];
	z = s->vt[2];
	m[1][0] = x;
	m[1][1] = y;
	m[1][2] = z;
	m[1][3] = - cx*x - cy*y - cz*z;

	x = s->vf[0];
	y = s->vf[1];
	z = s->vf[2];
	m[2][0] = x;
	m[2][1] = y;
	m[2][2] = z;
	m[2][3] = - cx*x - cy*y - cz*z;

	m[3][0] = 0.0;
	m[3][1] = 0.0;
	m[3][2] = 0.0;
	m[3][3] = 1.0;
}
void view2clip_projznzp(mat4 proj, struct camera* sty)
{
	float l = sty->vl[3];
	float r = sty->vr[3];
	float b = sty->vb[3];
	float t = sty->vt[3];
	float n = sty->vn[3];
	float f = sty->vf[3];
	//say("%f,%f,%f,%f,%f,%f\n",l,r,b,t,n,f);

	proj[0][0] = 2 * n / (r-l);
	proj[0][1] = 0.0;
	proj[0][2] = (r+l) / (l-r);
	proj[0][3] = 0.0;

	proj[1][0] = 0.0;
	proj[1][1] = 2 * n / (t-b);
	proj[1][2] = (t+b) / (b-t);
	proj[1][3] = 0.0;

	proj[2][0] = 0.0;
	proj[2][1] = 0.0;
	proj[2][2] = (n+f) / (f-n);
	proj[2][3] = 2*f*n / (n-f);

	proj[3][0] = 0.0;
	proj[3][1] = 0.0;
	proj[3][2] = 1.0;
	proj[3][3] = 0.0;
}

void mat4_multiplyfrom(float* o, float* u, float* v)
{
	o[ 0] = u[ 0]*v[ 0] + u[ 1]*v[ 4] + u[ 2]*v[ 8] + u[ 3]*v[12];
	o[ 1] = u[ 0]*v[ 1] + u[ 1]*v[ 5] + u[ 2]*v[ 9] + u[ 3]*v[13];
	o[ 2] = u[ 0]*v[ 2] + u[ 1]*v[ 6] + u[ 2]*v[10] + u[ 3]*v[14];
	o[ 3] = u[ 0]*v[ 3] + u[ 1]*v[ 7] + u[ 2]*v[11] + u[ 3]*v[15];

	o[ 4] = u[ 4]*v[ 0] + u[ 5]*v[ 4] + u[ 6]*v[ 8] + u[ 7]*v[12];
	o[ 5] = u[ 4]*v[ 1] + u[ 5]*v[ 5] + u[ 6]*v[ 9] + u[ 7]*v[13];
	o[ 6] = u[ 4]*v[ 2] + u[ 5]*v[ 6] + u[ 6]*v[10] + u[ 7]*v[14];
	o[ 7] = u[ 4]*v[ 3] + u[ 5]*v[ 7] + u[ 6]*v[11] + u[ 7]*v[15];

	o[ 8] = u[ 8]*v[ 0] + u[ 9]*v[ 4] + u[10]*v[ 8] + u[11]*v[12];
	o[ 9] = u[ 8]*v[ 1] + u[ 9]*v[ 5] + u[10]*v[ 9] + u[11]*v[13];
	o[10] = u[ 8]*v[ 2] + u[ 9]*v[ 6] + u[10]*v[10] + u[11]*v[14];
	o[11] = u[ 8]*v[ 3] + u[ 9]*v[ 7] + u[10]*v[11] + u[11]*v[15];

	o[12] = u[12]*v[ 0] + u[13]*v[ 4] + u[14]*v[ 8] + u[15]*v[12];
	o[13] = u[12]*v[ 1] + u[13]*v[ 5] + u[14]*v[ 9] + u[15]*v[13];
	o[14] = u[12]*v[ 2] + u[13]*v[ 6] + u[14]*v[10] + u[15]*v[14];
	o[15] = u[12]*v[ 3] + u[13]*v[ 7] + u[14]*v[11] + u[15]*v[15];
}
void mat4_multiply(float* l, float* r)
{
	int j;
	float t[16];
	for(j=0;j<16;j++)t[j] = l[j];
	mat4_multiplyfrom(l, t, r);
}
void mat4_transposefrom(float* m, float* u)
{
	m[ 0] = u[ 0];
	m[ 1] = u[ 4];
	m[ 2] = u[ 8];
	m[ 3] = u[12];

	m[ 4] = u[ 1];
	m[ 5] = u[ 5];
	m[ 6] = u[ 9];
	m[ 7] = u[13];

	m[ 8] = u[ 2];
	m[ 9] = u[ 6];
	m[10] = u[10];
	m[11] = u[14];

	m[12] = u[ 3];
	m[13] = u[ 7];
	m[14] = u[11];
	m[15] = u[15];
}
void mat4_transpose(float* u)
{
	float t;

	t = u[1];
	u[1] = u[4];
	u[4] = t;

	t = u[2];
	u[2] = u[8];
	u[8] = t;

	t = u[3];
	u[3] = u[12];
	u[12] = t;

	t = u[6];
	u[6] = u[9];
	u[9] = t;

	t = u[7];
	u[7] = u[13];
	u[13] = t;

	t = u[11];
	u[11] = u[14];
	u[14] = t;
}
void world2clip_projznzp_transpose(mat4 mat, struct camera* frus)
{
	mat4 t;
	world2view_rh2lh(t, frus);
	view2clip_projznzp(mat, frus);

	mat4_multiply((void*)mat, (void*)t);
	mat4_transpose((void*)mat);
}




void genrectfrompoint(struct vertex* dst, struct vertex* src)
{
	int j;
	for(j=0;j<6;j++){
		dst[j].r = src->r;
		dst[j].g = src->g;
		dst[j].b = src->b;
	}
	//l,b
	dst[0].x = src->x - cam.vr[0]*64 - cam.vt[0]*8;
	dst[0].y = src->y - cam.vr[1]*64 - cam.vt[1]*8;
	dst[0].z = src->z - cam.vr[2]*64 - cam.vt[2]*8;
	//r,b
	dst[1].x = src->x + cam.vr[0]*64 - cam.vt[0]*8;
	dst[1].y = src->y + cam.vr[1]*64 - cam.vt[1]*8;
	dst[1].z = src->z + cam.vr[2]*64 - cam.vt[2]*8;
	//r,t
	dst[2].x = src->x + cam.vr[0]*64 + cam.vt[0]*8;
	dst[2].y = src->y + cam.vr[1]*64 + cam.vt[1]*8;
	dst[2].z = src->z + cam.vr[2]*64 + cam.vt[2]*8;
	//r,t
	dst[3].x = src->x + cam.vr[0]*64 + cam.vt[0]*8;
	dst[3].y = src->y + cam.vr[1]*64 + cam.vt[1]*8;
	dst[3].z = src->z + cam.vr[2]*64 + cam.vt[2]*8;
	//l,t
	dst[4].x = src->x - cam.vr[0]*64 + cam.vt[0]*8;
	dst[4].y = src->y - cam.vr[1]*64 + cam.vt[1]*8;
	dst[4].z = src->z - cam.vr[2]*64 + cam.vt[2]*8;
	//l,b
	dst[5].x = src->x - cam.vr[0]*64 - cam.vt[0]*8;
	dst[5].y = src->y - cam.vr[1]*64 - cam.vt[1]*8;
	dst[5].z = src->z - cam.vr[2]*64 - cam.vt[2]*8;
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
	int j;
	forcedirected_3d(tbuf,tcnt, vbuf,vcnt, ibuf,icnt);
	vbuf[0].x = vbuf[0].y = vbuf[0].z = 0.0;

	for(j=0;j<vcnt;j++)genrectfrompoint(&node_vbuf[j*6+0], &vbuf[j]);


	int x0 = 0;
	int y0 = 0;
	int fbw;
	int fbh;
	glfwGetFramebufferSize(fw, &fbw, &fbh);

	cam.vt[3] = (float)fbh / (float)fbw;
	world2clip_projznzp_transpose(wvp, &cam);


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

	//wire's vao,vbo,ibo
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(struct vertex)*vcnt, vbuf);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	glDrawElements(GL_LINES, 4*icnt, GL_UNSIGNED_SHORT, (void*)0);

	//node's vao,vbo
	glBindVertexArray(nodevao);

	glBindBuffer(GL_ARRAY_BUFFER, nodevbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 6*sizeof(struct vertex)*vcnt, node_vbuf);

	glDrawArrays(GL_TRIANGLES, 0, 6*vcnt);
}




void glfw_initdata(struct pernode* nb, int nl, struct perwire* wb, int wl)
{
	int vlen = nl * sizeof(struct vertex);
	int ilen = wl * 4;
	tcnt = nl;
	tbuf = malloc(vlen);
	vcnt = nl;
	vbuf = malloc(vlen);
	icnt = wl;
	ibuf = (void*)wb;

	int j;
	for(j=0;j<nl;j++){
		vbuf[j].x = (rand()%20000)/10000.0 - 1.0;
		vbuf[j].y = (rand()%20000)/10000.0 - 1.0;
		vbuf[j].z = (rand()%20000)/10000.0 - 1.0;
		switch(nb[j].type){
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

	//wire data
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


	//node data
	node_vbuf = malloc(vlen*6);

	glGenVertexArrays(1, &nodevao);
	glBindVertexArray(nodevao);

	glGenBuffers(1, &nodevbo);
	glBindBuffer(GL_ARRAY_BUFFER, nodevbo);
	glBufferData(GL_ARRAY_BUFFER, vlen*6, node_vbuf, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, (void*)12);
	glEnableVertexAttribArray(1);


	//camera geometry
	cam.vc[0] = 0.0;
	cam.vc[1] =-1000.0;
	cam.vc[2] = 0.0;
	cam.vc[3] = 0.0;
	cam.vr[0] = 1.0;
	cam.vr[1] = 0.0;
	cam.vr[2] = 0.0;
	cam.vr[3] = 1.0;
	cam.vl[0] = cam.vr[0];
	cam.vl[1] = cam.vr[1];
	cam.vl[2] = cam.vr[2];
	cam.vl[3] =-cam.vr[3];
	cam.vf[0] = 0.0;
	cam.vf[1] = 1.0;
	cam.vf[2] = 0.0;
	cam.vf[3] = 10000000.0;
	cam.vn[0] = cam.vf[0];
	cam.vn[1] = cam.vf[1];
	cam.vn[2] = cam.vf[2];
	cam.vn[3] = 1.0;
	cam.vt[0] = 0.0;
	cam.vt[1] = 0.0;
	cam.vt[2] = 1.0;
	cam.vt[3] = 1.0;
	cam.vb[0] = cam.vt[0];
	cam.vb[1] = cam.vt[1];
	cam.vb[2] = cam.vt[2];
	cam.vb[3] =-cam.vt[3];
}
void glfw_freedata()
{
	glDeleteBuffers(1, &nodevbo);
	glDeleteBuffers(1, &nodevao);
	glDeleteVertexArrays(1, &nodevao);

	glDeleteBuffers(1, &ibo);
	glDeleteBuffers(1, &vbo);
	glDeleteBuffers(1, &vao);
	glDeleteVertexArrays(1, &vao);

	glDeleteProgram(shader);

	free(tbuf);
	free(vbuf);
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




void render_data(struct pernode* nb, int cl, struct perwire* wb, int wl)
{
	printf("nodecnt=%d,wirecnt=%d\n", cl, wl);

	GLFWwindow* fw = glfw_initwindow(1024, 768);
	if(0 == fw)return;

	glfw_initdata(nb,cl, wb,wl);

	while(1){
		glfwMakeContextCurrent(fw);

		wvp[0][0] = 1.0;wvp[0][1] = 0.0;wvp[0][2] = 0.0;wvp[0][3] = 0.0;
		wvp[1][0] = 0.0;wvp[1][1] = 1.0;wvp[1][2] = 0.0;wvp[1][3] = 0.0;
		wvp[2][0] = 0.0;wvp[2][1] = 0.0;wvp[2][2] = 1.0;wvp[2][3] = 0.0;
		wvp[3][0] = 0.0;wvp[3][1] = 0.0;wvp[3][2] = 0.0;wvp[3][3] = 1.0;

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
void render_init(void* nb, int nl, void* wb, int wl)
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
