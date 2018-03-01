#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define PI 3.1415926535897932384626433832795028841971693993151
#define hex32(a,b,c,d) (a | (b<<8) | (c<<16) | (d<<24))
#define hex64(a,b,c,d,e,f,g,h) (hex32(a,b,c,d) | (((u64)hex32(e,f,g,h))<<32))
#define _hash_ hex32('h','a','s','h')
#define _file_ hex32('f','i','l','e')
#define _func_ hex32('f','u','n','c')
#define _chip_ hex32('c','h','i','p')
#define _pin_ hex32('p','i','n',0)
#define _shape_ hex32('s','h','a','p')
void vectornormalize(float* v);
void vectorcross(float* v, float* x);
void quaternionrotate(float* v, float* q);
//
void drawascii(void*, u32, int, int, int, int, u8);
void drawascii_alpha(void*, int, int, int, int, u8);
//
void carveshape(void* obj, u32 rgb, float x, float y, float z);
void carvestring(void* obj, u32 rgb, float x, float y, float z, u8* buf, int len);
void forcedirected_3d(void*, int, void*, int, void*, int);




//
static int width;
static int height;
static int enqueue = 0;
static int dequeue = 0;
static int last_x = 0;
static int last_y = 0;
//
static GLuint simpleprogram;
static GLuint prettyprogram;
static GLuint myfontprogram;
//
static float light0[4] = {0.0f, 0.0f, 10.0f};
static float camera[4] = {1.0f, -2.0f, 1.0f};
static float center[4] = {0.0f, 0.0f, 0.0f};
static float above[4] = {0.0f, 0.0f, 1.0f};
//
static GLfloat modelmatrix[4*4] = {
	1.0f, 0.0f, 0.0f, 0.0f,  
	0.0f, 1.0f, 0.0f, 0.0f,  
	0.0f, 0.0f, 1.0f, 0.0f,  
	0.0f, 0.0f, 0.0f, 1.0f,  
};
static GLfloat viewmatrix[4*4] = {
	1.0f, 0.0f, 0.0f, 0.0f,  
	0.0f, 1.0f, 0.0f, 0.0f,  
	0.0f, 0.0f, 1.0f, 0.0f,  
	0.0f, 0.0f, 0.0f, 1.0f,  
};
static GLfloat projmatrix[4*4] = {
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, -1.0f, -1.0f,
	0.0f, 0.0f, -0.2f, 0.0f
};
static GLfloat cameramvp[4*4];
static GLfloat light0mvp[4*4];
//
struct object
{
	u32 vbo;
	u32 len;
	void* buf;
};
struct eachone
{
	u32 vao;
};
static struct object obj[8];
static struct eachone each[4];
//
struct context
{
	u64 type;
	u64 addr;
	u8 str[16];
};
static struct context* ctxbuf = 0;
static int ctxlen = 0;
//
static u8 fontdata[128*128];
static u8 utf8data;




char simplevert[] = {
	"#version 300 es\n"
	"layout(location = 0)in mediump vec3 position;\n"
	"layout(location = 1)in mediump vec3 color;\n"
	"uniform mat4 cameramvp;\n"
	"out mediump vec3 vertexcolor;\n"
	"void main()\n"
	"{\n"
		"vertexcolor = color;\n"
		"gl_Position = cameramvp * vec4(position,1.0);\n"
	"}\n"
};
char simplefrag[] = {
	"#version 300 es\n"
	"in mediump vec3 vertexcolor;\n"
	"out mediump vec4 FragColor;\n"
	"void main()\n"
	"{\n"
		"FragColor = vec4(vertexcolor,1.0);\n"
	"}\n"
};
char prettyvert[] = {
	"#version 300 es\n"
	"layout(location = 0)in mediump vec3 position;\n"
	"layout(location = 1)in mediump vec3 color;\n"
	"layout(location = 2)in mediump vec3 normal;\n"
	"uniform mat4 cameramvp;\n"
	"uniform mat4 shadowmvp;\n"
	"uniform mediump vec3 ambientcolor;\n"
	"uniform mediump vec3 lightcolor;\n"
	"uniform mediump vec3 lightposition;\n"
	"uniform mediump vec3 eyeposition;\n"
	"out mediump vec3 vertexcolor;\n"
	"void main()\n"
	"{\n"
		"mediump vec3 N = normalize(normal);\n"
		"mediump vec3 L = normalize(vec3(lightposition - position));\n"
		"mediump vec3 E = normalize(eyeposition-position);\n"
		"mediump vec3 R = reflect(-L, N);\n"
		"mediump float SN = max(dot(N, L), 0.0);\n"
		"mediump float RV = max(dot(R, E), 0.0);\n"
		"mediump vec3 ambient = color * ambientcolor;\n"
		"mediump vec3 diffuse = color * lightcolor * SN;\n"
		"mediump vec3 specular = vec3(0.0, 0.0, 0.0);\n"
		"if(SN>0.0)specular = color * lightcolor * pow(RV, 8.0);\n"
		"vertexcolor = ambient + diffuse + specular;\n"
		"gl_Position = cameramvp * vec4(position,1.0);\n"
	"}\n"
};
char prettyfrag[] = {
	"#version 300 es\n"
	"in mediump vec3 vertexcolor;\n"
	"out mediump vec4 FragColor;\n"
	"void main()\n"
	"{\n"
		"FragColor = vec4(vertexcolor,1.0);\n"
	"}\n"
};
char myfontvert[] = {
	"#version 300 es\n"
	"layout(location = 0)in mediump vec3 position;\n"
	"layout(location = 1)in mediump vec3 color;\n"
	"layout(location = 2)in mediump vec2 texcoord;\n"
	"uniform mat4 cameramvp;\n"
	"out mediump vec3 origcolor;\n"
	"out mediump vec2 texuv;\n"
	"void main()\n"
	"{\n"
		"gl_Position = cameramvp * vec4(position,1.0);\n"
		"origcolor = color;\n"
		"texuv = texcoord;\n"
	"}\n"
};
char myfontfrag[] = {
	"#version 300 es\n"
	"in mediump vec3 origcolor;\n"
	"in mediump vec2 texuv;\n"
	"uniform sampler2D texdata;\n"
	"out mediump vec4 FragColor;\n"
	"void main()\n"
	"{\n"
		"FragColor = vec4(origcolor,1.0)*texture(texdata, texuv).aaaa;\n"
	"}\n"
};
void initshader_one(GLuint* prog, void* vert, void* frag)
{
	//1.vertex shader
	GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
	if(0 == vShader)
	{
		printf("ERROR : Create vertex shader failed\n");
		exit(1);
	}

	const GLchar* vCodeArray[1] = {vert};
	glShaderSource(vShader, 1, vCodeArray, NULL);
	glCompileShader(vShader);

	GLint compileResult;
	glGetShaderiv(vShader, GL_COMPILE_STATUS, &compileResult);
	if (GL_FALSE == compileResult)
	{
		GLint logLen;
		//得到编译日志长度
		glGetShaderiv(vShader, GL_INFO_LOG_LENGTH, &logLen);
		if (logLen > 0)
		{
			GLsizei written;
			char *log = (char*)malloc(logLen);

			//得到日志信息并输出
			glGetShaderInfoLog(vShader, logLen, &written, log);
			printf("vertex shader compile log: %s\n",log);
			free(log);
		}
	}

	//2.fragment shader
	GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
	if (0 == fShader)
	{
		printf("ERROR : Create fragment shader failed");
		exit(1);
	}

	const GLchar* fCodeArray[1] = {frag};
	glShaderSource(fShader, 1, fCodeArray, NULL);
	glCompileShader(fShader);

	glGetShaderiv(fShader, GL_COMPILE_STATUS, &compileResult);
	if(GL_FALSE == compileResult)
	{
		GLint logLen;
		glGetShaderiv(fShader, GL_INFO_LOG_LENGTH, &logLen);
		if(logLen > 0)
		{
			GLsizei written;
			char *log = (char*)malloc(logLen);

			glGetShaderInfoLog(fShader, logLen, &written, log);
			printf("fragment shader compile log: %s\n",log);
			free(log);
		}
	}

	//3.glsl program
	*prog = glCreateProgram();
	if(*prog == 0)
	{
		printf("ERROR : create program failed");
		exit(1);
	}

	glAttachShader(*prog, vShader);
	glAttachShader(*prog, fShader);
	glLinkProgram(*prog);

	GLint linkStatus;
	glGetProgramiv(*prog, GL_LINK_STATUS, &linkStatus);
	if(GL_FALSE == linkStatus)
	{
		printf("ERROR : link shader program failed");
		GLint logLen;
		glGetProgramiv(*prog, GL_INFO_LOG_LENGTH, &logLen);
		if(logLen > 0)
		{
			char *log = (char*)malloc(logLen);
			GLsizei written;
			glGetProgramInfoLog(*prog, logLen, &written, log);
			printf("Program log :%s\n", log);
		}
	}
}
void initshader()
{
	const GLubyte *renderer = glGetString(GL_RENDERER);
	const GLubyte *vendor = glGetString(GL_VENDOR);
	const GLubyte *version = glGetString(GL_VERSION);
	const GLubyte *glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
	GLint major, minor;
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	//printf("GL Vendor: %s\n", vendor);
	//printf("GL Renderer: %s\n", renderer);
	//printf("GL Version (string): %s\n", version);
	//printf("GLSL Version: %s\n", glslVersion);
	//printf("GL Version (integer): %x.%x\n", major, minor);

	initshader_one(&simpleprogram, simplevert, simplefrag);
	initshader_one(&prettyprogram, prettyvert, prettyfrag);
	initshader_one(&myfontprogram, myfontvert, myfontfrag);
	glUseProgram(prettyprogram);
}
void inittexture()
{
	int j;
	u8 ch;
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);  //GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);  //GL_REPEAT);

	for(ch=0x20;ch<0x80;ch++)
	{
		drawascii_alpha(
			fontdata, 128, 128,
			(ch&0xf)<<3, (ch&0xf0)-32, ch
		);
	}
	glTexImage2D(GL_TEXTURE_2D, 0,
		GL_ALPHA, 8*16, 16*8, 0,
		GL_ALPHA, GL_UNSIGNED_BYTE, fontdata
	);
}
void initobject()
{
        //point: 1=obj(vertex,color)
        glGenBuffers(1, &obj[1].vbo);
        glBindBuffer(GL_ARRAY_BUFFER, obj[1].vbo);
        glBufferData(GL_ARRAY_BUFFER, 0x200000, obj[1].buf, GL_STATIC_DRAW);


        //line: 2=ibo, 3=obj(vertex,color)
        glGenBuffers(1, &obj[2].vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj[2].vbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0x200000, obj[2].buf, GL_STATIC_DRAW);

        glGenBuffers(1, &obj[3].vbo);
        glBindBuffer(GL_ARRAY_BUFFER, obj[3].vbo);
        glBufferData(GL_ARRAY_BUFFER, 0x200000, obj[3].buf, GL_STATIC_DRAW);


        //trigon: 4=ibo, 5=obj(vertex,color,normal)
        glGenBuffers(1, &obj[4].vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj[4].vbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0x200000, obj[4].buf, GL_STATIC_DRAW);

        glGenBuffers(1, &obj[5].vbo);
        glBindBuffer(GL_ARRAY_BUFFER, obj[5].vbo);
        glBufferData(GL_ARRAY_BUFFER, 0x200000, obj[5].buf, GL_STATIC_DRAW);


        //font: 6=ibo, 7=obj(vertex,color,texcoor)
        glGenBuffers(1, &obj[6].vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj[6].vbo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0x200000, obj[6].buf, GL_STATIC_DRAW);

        glGenBuffers(1, &obj[7].vbo);
        glBindBuffer(GL_ARRAY_BUFFER, obj[7].vbo);
        glBufferData(GL_ARRAY_BUFFER, 0x200000, obj[7].buf, GL_STATIC_DRAW);

	//point
        glGenVertexArrays(1, &each[0].vao);
        glBindVertexArray(each[0].vao);

        glBindBuffer(GL_ARRAY_BUFFER, obj[1].vbo);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, (void*)12);
        glEnableVertexAttribArray(1);

        //line
        glGenVertexArrays(1, &each[1].vao);
        glBindVertexArray(each[1].vao);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj[2].vbo);
        glBindBuffer(GL_ARRAY_BUFFER, obj[3].vbo);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 24, (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 24, (void*)12);
        glEnableVertexAttribArray(1);

        //trigon
        glGenVertexArrays(1, &each[2].vao);
        glBindVertexArray(each[2].vao);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj[4].vbo);
        glBindBuffer(GL_ARRAY_BUFFER, obj[5].vbo);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 36, (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 36, (void*)12);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 36, (void*)24);
        glEnableVertexAttribArray(2);

        //font
        glGenVertexArrays(1, &each[3].vao);
        glBindVertexArray(each[3].vao);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj[6].vbo);
        glBindBuffer(GL_ARRAY_BUFFER, obj[7].vbo);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 36, (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 36, (void*)12);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 36, (void*)24);
        glEnableVertexAttribArray(2);
}




void matrixmultiply(GLfloat* u, GLfloat* v)
{
	int j;
	float w[16];
	for(j=0;j<16;j++)w[j] = u[j];

	u[ 0] = w[ 0]*v[ 0] + w[ 1]*v[ 4] + w[ 2]*v[ 8] + w[ 3]*v[12];
	u[ 1] = w[ 0]*v[ 1] + w[ 1]*v[ 5] + w[ 2]*v[ 9] + w[ 3]*v[13];
	u[ 2] = w[ 0]*v[ 2] + w[ 1]*v[ 6] + w[ 2]*v[10] + w[ 3]*v[14];
	u[ 3] = w[ 0]*v[ 3] + w[ 1]*v[ 7] + w[ 2]*v[11] + w[ 3]*v[15];

	u[ 4] = w[ 4]*v[ 0] + w[ 5]*v[ 4] + w[ 6]*v[ 8] + w[ 7]*v[12];
	u[ 5] = w[ 4]*v[ 1] + w[ 5]*v[ 5] + w[ 6]*v[ 9] + w[ 7]*v[13];
	u[ 6] = w[ 4]*v[ 2] + w[ 5]*v[ 6] + w[ 6]*v[10] + w[ 7]*v[14];
	u[ 7] = w[ 4]*v[ 3] + w[ 5]*v[ 7] + w[ 6]*v[11] + w[ 7]*v[15];

	u[ 8] = w[ 8]*v[ 0] + w[ 9]*v[ 4] + w[10]*v[ 8] + w[11]*v[12];
	u[ 9] = w[ 8]*v[ 1] + w[ 9]*v[ 5] + w[10]*v[ 9] + w[11]*v[13];
	u[10] = w[ 8]*v[ 2] + w[ 9]*v[ 6] + w[10]*v[10] + w[11]*v[14];
	u[11] = w[ 8]*v[ 3] + w[ 9]*v[ 7] + w[10]*v[11] + w[11]*v[15];

	u[12] = w[12]*v[ 0] + w[13]*v[ 4] + w[14]*v[ 8] + w[15]*v[12];
	u[13] = w[12]*v[ 1] + w[13]*v[ 5] + w[14]*v[ 9] + w[15]*v[13];
	u[14] = w[12]*v[ 2] + w[13]*v[ 6] + w[14]*v[10] + w[15]*v[14];
	u[15] = w[12]*v[ 3] + w[13]*v[ 7] + w[14]*v[11] + w[15]*v[15];
}
void fixmodel()
{
	//matrix = movematrix * rotatematrix * scalematrix
}
void fixview()
{
	//a X b = [ay*bz - az*by, az*bx-ax*bz, ax*by-ay*bx]
	float norm;

	//Z = center - camera
	float nx = center[0] - camera[0];
	float ny = center[1] - camera[1];
	float nz = center[2] - camera[2];
	norm = sqrt(nx*nx + ny*ny + nz*nz);
	nx /= norm;
	ny /= norm;
	nz /= norm;

	//X = cross(Z, above)
	float ux = ny*above[2] - nz*above[1];
	float uy = nz*above[0] - nx*above[2];
	float uz = nx*above[1] - ny*above[0];
	norm = sqrt(ux*ux + uy*uy + uz*uz);
	ux /= norm;
	uy /= norm;
	uz /= norm;

	//above = cross(X, Z)
	float vx = uy*nz - uz*ny;
	float vy = uz*nx - ux*nz;
	float vz = ux*ny - uy*nx;

	viewmatrix[0] = ux;
	viewmatrix[1] = vx;
	viewmatrix[2] = -nx;
	viewmatrix[3] = 0.0f;

	viewmatrix[4] = uy;
	viewmatrix[5] = vy;
	viewmatrix[6] = -ny;
	viewmatrix[7] = 0.0f;

	viewmatrix[8] = uz;
	viewmatrix[9] = vz;
	viewmatrix[10] = -nz;
	viewmatrix[11] = 0.0f;

	viewmatrix[12] = -camera[0]*ux - camera[1]*uy - camera[2]*uz;
	viewmatrix[13] = -camera[0]*vx - camera[1]*vy - camera[2]*vz;
	viewmatrix[14] = camera[0]*nx + camera[1]*ny + camera[2]*nz;
	viewmatrix[15] = 1.0f;
}
void fixprojection()
{
/*
	cot45, 0, 0, 0,
	0, cot45, 0, 0,
	0, 0, (f+n)/(f-n), -1,
	0, 0, (2*f*n)/(f-n), 0
*/
	float w = (float)width;
	float h = (float)height;
	glViewport(0, 0, width, height);
	projmatrix[0] = h / w;
}
void fixmatrix()
{
	int x;

	fixmodel();
	fixview();
	fixprojection();

	for(x=0;x<16;x++)cameramvp[x] = modelmatrix[x];
	matrixmultiply(cameramvp, viewmatrix);
	matrixmultiply(cameramvp, projmatrix);
}
void fixlight()
{
	GLfloat ambientcolor[3] = {0.5f, 0.5f, 0.5f};
	GLfloat lightcolor[3] = {1.0f, 1.0f, 1.0f};

	GLint ac = glGetUniformLocation(prettyprogram, "ambientcolor");
	glUniform3fv(ac, 1, ambientcolor);

	GLint dc = glGetUniformLocation(prettyprogram, "lightcolor");
	glUniform3fv(dc, 1, lightcolor);

	GLint dp = glGetUniformLocation(prettyprogram, "lightposition");
	glUniform3fv(dp, 1, light0);

	GLint ep = glGetUniformLocation(prettyprogram, "eyeposition");
	glUniform3fv(ep, 1, camera);
}
void callback_display()
{
	//set
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//
	fixmatrix();
	fixlight();

	//no light
	glUseProgram(simpleprogram);
	GLint mvp1 = glGetUniformLocation(simpleprogram, "cameramvp");
	glUniformMatrix4fv(mvp1, 1, GL_FALSE, cameramvp);

	glBindVertexArray(each[0].vao);
	glDrawArrays(GL_POINTS, 0, obj[1].len);

	glBindVertexArray(each[1].vao);
	glDrawElements(GL_LINES, 2*obj[2].len, GL_UNSIGNED_SHORT, 0);

	//have light
	glUseProgram(prettyprogram);
	GLint mvp2 = glGetUniformLocation(prettyprogram, "cameramvp");
	glUniformMatrix4fv(mvp2, 1, GL_FALSE, cameramvp);

	glBindVertexArray(each[2].vao);
	glDrawElements(GL_TRIANGLES, 3*obj[4].len, GL_UNSIGNED_SHORT, 0);

	//utf8
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(myfontprogram);

	GLint mvp3 = glGetUniformLocation(myfontprogram, "cameramvp");
	glUniformMatrix4fv(mvp3, 1, GL_FALSE, cameramvp);
	GLint tex = glGetUniformLocation(myfontprogram, "texdata");
	glUniform1i(tex, 0);

	glBindVertexArray(each[3].vao);
	glDrawElements(GL_TRIANGLES, 3*obj[6].len, GL_UNSIGNED_SHORT, 0);

	glDisable(GL_BLEND);
	glDepthMask(GL_TRUE);

	//write
	glFlush();
	glutSwapBuffers();
}




void graph_hack(struct context* ctxbuf, int ctxlen)
{
	int j,k;
	u32 c;
	int llen = obj[2].len;
	u16* lbuf = obj[2].buf;
	int vlen = obj[3].len;
	float* vbuf = obj[3].buf;
	float* obuf = obj[5].buf;

	forcedirected_3d(obuf, vlen, vbuf, vlen, lbuf, llen);
	vbuf[0] = vbuf[1] = vbuf[2] = 0.0;
	vbuf[6] = vbuf[7] = vbuf[8] = 0.0;

	obj[4].len = 0;
	obj[5].len = 0;
	obj[6].len = 0;
	obj[7].len = 0;
	for(j=0;j<ctxlen;j++)
	{
		if(_hash_ == ctxbuf[j].type)c=0xff0000;
		else if(_file_ == ctxbuf[j].type)c = 0x00ff00;
		else if(_func_ == ctxbuf[j].type)c = 0x0000ff;
		else if(_chip_ == ctxbuf[j].type)c = 0x6619b3;
		else if( _pin_ == ctxbuf[j].type)c = 0xcd4dff;
		else c = 0xe68019;

		if(hex32('h','a','s','h') == ctxbuf[j].type)
		{
			k = 12*j;
			carvestring(
				obj, c,
				vbuf[k+0], vbuf[k+1], vbuf[k+2],
				ctxbuf[j].str, 0
			);
		}
		else
		{
			k = 12*j;
			carveshape(
				obj, c,
				vbuf[k+0], vbuf[k+1], vbuf[k+2]
			);
		}
	}
}
void callback_idle()
{
	if(enqueue != dequeue)dequeue = (dequeue+1)%0x10000;
	else graph_hack(ctxbuf, ctxlen);

	glBindBuffer(GL_ARRAY_BUFFER, obj[1].vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 24*obj[1].len, obj[1].buf);

	glBindBuffer(GL_ARRAY_BUFFER, obj[3].vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 24*obj[3].len, obj[3].buf);

	glBindBuffer(GL_ARRAY_BUFFER, obj[5].vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 36*obj[5].len, obj[5].buf);

	glBindBuffer(GL_ARRAY_BUFFER, obj[7].vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, 36*obj[7].len, obj[7].buf);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj[2].vbo);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 4*obj[2].len, obj[2].buf);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj[4].vbo);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 6*obj[4].len, obj[4].buf);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, obj[6].vbo);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 6*obj[6].len, obj[6].buf);

	glutPostRedisplay();
}
void callback_keyboard(unsigned char key,int x,int y)
{
	printf("%d\n",key);
	glutPostRedisplay();
}
void callback_reshape(int w, int h)
{
	width = w;
	height = h;
}
void callback_move(int x,int y)
{
	float t[3];
	float v[4];
	t[0] = 0.0;
	t[1] = 0.0;
	t[2] = 1.0;
	v[0] = camera[0];
	v[1] = camera[1];
	v[2] = camera[2];
	v[3] = 0.0;
	if(x>last_x)
	{
		camera[0] = v[0]*cos(0.05f) + v[1]*sin(0.05f);
		camera[1] = -v[0]*sin(0.05f) + v[1]*cos(0.05f);

		//camera_yaw += PI/90;
	}
	else if(x<last_x)
	{
		camera[0] = v[0]*cos(0.05f) - v[1]*sin(0.05f);
		camera[1] = v[0]*sin(0.05f) + v[1]*cos(0.05f);

		//camera_yaw -= PI/90;
	}

	if(y > last_y)
	{
		vectorcross(v, t);
		vectornormalize(v);

		v[0] *= sin(0.02f);
		v[1] *= sin(0.02f);
		v[2] *= sin(0.02f);
		v[3] = cos(0.02f);
		quaternionrotate(camera, v);
	}
	else if(y<last_y)
	{
		vectorcross(v, t);
		vectornormalize(v);

		v[0] *= sin(-0.02f);
		v[1] *= sin(-0.02f);
		v[2] *= sin(-0.02f);
		v[3] = cos(-0.02f);
		quaternionrotate(camera, v);
	}

	last_x = x;
	last_y = y;
	glutPostRedisplay();
}
void callback_mouse(int button, int state, int x, int y)
{
	float tx, ty, tz;
	if(state == GLUT_DOWN)
	{
		last_x = x;
		last_y = y;
	}
	if(state == GLUT_UP)
	{
		tx = camera[0];
		ty = camera[1];
		tz = camera[2];
		if(button == 3)	//wheel_up
		{
			camera[0] = 0.9*tx + 0.1*center[0];
			camera[1] = 0.9*ty + 0.1*center[1];
			camera[2] = 0.9*tz + 0.1*center[2];

			//camera_zoom *= 0.95;
			glutPostRedisplay();
		}
		else if(button == 4)	//wheel_down
		{
			camera[0] = 1.1*tx - 0.1*center[0];
			camera[1] = 1.1*ty - 0.1*center[1];
			camera[2] = 1.1*tz - 0.1*center[2];

			//camera_zoom *= 1.05263158;
			glutPostRedisplay();
		}
	}
}
void* graph_thread(void* arg)
{
	int err;
	int argc = 1;
	char* argv[2] = {"a.out", 0};
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutInitWindowPosition(256, 256);
	glutCreateWindow("evil");

	err = glewInit();
	if( GLEW_OK != err )printf("glewinit: %s\n", glewGetErrorString(err));  

	glPointSize(2.0);
	glViewport(0, 0, 512, 512);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	initshader();
	inittexture();
	initobject();

	glutIdleFunc(callback_idle);
	glutDisplayFunc(callback_display);
	glutKeyboardFunc(callback_keyboard);
	glutReshapeFunc(callback_reshape);
	glutMouseFunc(callback_mouse);
	glutMotionFunc(callback_move); 
	  
	glutMainLoop();

	glUseProgram(0);
	return 0;
}
void graph_init(void* cb, int cl, void* lb, int ll)
{
	ctxbuf = cb;
	ctxlen = cl;

	//point
	obj[0].len = 0;
	obj[0].buf = 0;
	obj[1].len = 0;
	obj[1].buf = malloc(0x200000);

	//line
	obj[2].len = 0;
	obj[2].buf = lb;
	obj[3].len = 0;
	obj[3].buf = malloc(0x200000);

	//trigon
	obj[4].len = 0;
	obj[4].buf = malloc(0x200000);
	obj[5].len = 0;
	obj[5].buf = malloc(0x200000);

	//utf8
	obj[6].len = 0;
	obj[6].buf = malloc(0x200000);
	obj[7].len = 0;
	obj[7].buf = malloc(0x200000);

	u64 id;
	pthread_create((void*)&id, NULL, graph_thread, 0);
}
void graph_data(void* cb, int cl, void* lb, int ll)
{
	int j;
	float r,g,b;
	float* vv;
	void* buf;

	enqueue = (enqueue+1)%0x10000;
	ctxbuf = cb;
	ctxlen = cl;
	obj[2].len = ll;
	obj[3].len = cl*2;

	buf = (void*)(obj[3].buf);
	for(j=0;j<ctxlen;j++)
	{
		vv = buf + 48*j;

		vv[ 0] = (rand()&0xffff) / 65536.0;
		vv[ 1] = (rand()&0xffff) / 65536.0;
		vv[ 2] = (rand()&0xffff) / 65536.0;

		if(     _hash_ == ctxbuf[j].type){r=1.0;g=0.0;b=0.0;}
		else if(_file_ == ctxbuf[j].type){r=0.0;g=1.0;b=0.0;}
		else if(_func_ == ctxbuf[j].type){r=0.0;g=0.0;b=1.0;}
		else if(_chip_ == ctxbuf[j].type){r=0.4;g=1.0;b=0.7;}
		else if( _pin_ == ctxbuf[j].type){r=0.8;g=0.3;b=1.0;}
		else {r=1.0;g=1.0;b=1.0;}
		vv[ 3] = r;
		vv[ 4] = g;
		vv[ 5] = b;

		vv[ 6] = 0.0;
		vv[ 7] = 0.0;
		vv[ 8] = 0.0;

		vv[ 9] = 0.1;
		vv[10] = 0.1;
		vv[11] = 0.1;
	}
}
