#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <pthread.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define PI 3.1415926535897932384626433832795028841971693993151
void vectornormalize(float* v);
void vectorcross(float* v, float* x);
void quaternionrotate(float* v, float* q);
void graph_hack(void*, void*, void*, int);




//
int width;
int height;
int enqueue = 0;
int dequeue = 0;
int last_x = 0;
int last_y = 0;
//
GLuint vShader;
GLuint fShader;
GLuint programHandle;
GLuint shapevao1;
GLuint shapevao2;
GLuint shapevao3;
GLuint shapevao4;
//
GLuint vertexhandle;
GLuint normalhandle;
GLuint colourhandle;
GLuint texturehandle;
GLuint indexhandle1;
GLuint indexhandle2;
GLuint indexhandle3;
GLuint indexhandle4;
//
float camera[4] = {1.0f, -2.0f, 1.0f};
float center[4] = {0.0f, 0.0f, 0.0f};
float above[4] = {0.0f, 0.0f, 1.0f};
//
GLfloat modelmatrix[4*4] = {  
	1.0f, 0.0f, 0.0f, 0.0f,  
	0.0f, 1.0f, 0.0f, 0.0f,  
	0.0f, 0.0f, 1.0f, 0.0f,  
	0.0f, 0.0f, 0.0f, 1.0f,  
};
GLfloat viewmatrix[4*4] = {  
	1.0f, 0.0f, 0.0f, 0.0f,  
	0.0f, 1.0f, 0.0f, 0.0f,  
	0.0f, 0.0f, 1.0f, 0.0f,  
	0.0f, 0.0f, 0.0f, 1.0f,  
};
GLfloat projmatrix[4*4] = {  
	1.0f, 0.0f, 0.0f, 0.0f,
	0.0f, 1.0f, 0.0f, 0.0f,
	0.0f, 0.0f, -1.0f, -1.0f,
	0.0f, 0.0f, -0.2f, 0.0f
};
//
struct binfo
{
        u64 vertexcount;
        u64 normalcount;
        u64 colorcount;
        u64 texturecount;
        u64 pointcount;
        u64 linecount;
        u64 tricount;
        u64 rectcount;
};
void* buffer = 0;
static struct binfo* binfo = 0;
static void* ctxbuf = 0;
static int ctxlen = 0;




char vCode[] = {
	"#version 300 es\n"
	"layout(location = 0)in mediump vec3 position;\n"
	"layout(location = 1)in mediump vec3 normal;\n"
	"layout(location = 2)in mediump vec3 color;\n"
	"uniform mediump vec3 ambientcolor;\n"
	"uniform mediump vec3 lightcolor;\n"
	"uniform mediump vec3 lightposition;\n"
	"uniform mediump vec3 eyeposition;\n"
	"uniform mat4 modelviewproj;\n"
	"uniform mat4 normalmatrix;\n"
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
		"gl_Position = modelviewproj * vec4(position,1.0);\n"
	"}\n"
};
char fCode[] = {
	"#version 300 es\n"
	"in mediump vec3 vertexcolor;\n"
	"out mediump vec4 FragColor;\n"
	"void main()\n"
	"{\n"
		"FragColor = vec4(vertexcolor,1.0);\n"
	"}\n"
};
void initshader()  
{  
	const GLubyte *renderer = glGetString( GL_RENDERER );  
	const GLubyte *vendor = glGetString( GL_VENDOR );  
	const GLubyte *version = glGetString( GL_VERSION );  
	const GLubyte *glslVersion = glGetString( GL_SHADING_LANGUAGE_VERSION );  
	GLint major, minor;  
	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	//printf("GL Vendor: %s\n", vendor);
	//printf("GL Renderer: %s\n", renderer);
	//printf("GL Version (string): %s\n", version);
	//printf("GLSL Version: %s\n", glslVersion);
	//printf("GL Version (integer): %x.%x\n", major, minor);

	vShader = glCreateShader(GL_VERTEX_SHADER);
	if (0 == vShader)  
	{  
		printf("ERROR : Create vertex shader failed\n");
		exit(1);  
	}  

	const GLchar* vCodeArray[1] = {vCode};
	glShaderSource(vShader, 1, vCodeArray, NULL);
	glCompileShader(vShader);  

	GLint compileResult;  
	glGetShaderiv(vShader,GL_COMPILE_STATUS,&compileResult);  
	if (GL_FALSE == compileResult)  
	{  
		GLint logLen;  
		glGetShaderiv(vShader,GL_INFO_LOG_LENGTH,&logLen);  
		if (logLen > 0)  
		{  
			GLsizei written;  
			char *log = (char *)malloc(logLen);  
			glGetShaderInfoLog(vShader,logLen,&written,log);
			printf("vertex shader compile log: %s\n",log);
			free(log);
		}
	}

	fShader = glCreateShader(GL_FRAGMENT_SHADER);
	if (0 == fShader)  
	{  
		printf("ERROR : Create fragment shader failed");  
		exit(1);  
	}  

	const GLchar* fCodeArray[1] = {fCode};
	glShaderSource(fShader, 1, fCodeArray, NULL);
	glCompileShader(fShader);  

	glGetShaderiv(fShader,GL_COMPILE_STATUS,&compileResult);  
	if (GL_FALSE == compileResult)  
	{  
		GLint logLen;  
		glGetShaderiv(fShader,GL_INFO_LOG_LENGTH,&logLen);  
		if (logLen > 0)  
		{  
			GLsizei written;  
			char *log = (char *)malloc(logLen);  
			glGetShaderInfoLog(fShader,logLen,&written,log);
			printf("fragment shader compile log: %s\n",log);
			free(log);
		}  
	}  
  
	programHandle = glCreateProgram();  
	if (!programHandle)  
	{  
		printf("ERROR : create program failed");
		exit(1);  
	}

	glAttachShader(programHandle,vShader);
	glAttachShader(programHandle,fShader);
	glLinkProgram(programHandle);

	GLint linkStatus;  
	glGetProgramiv(programHandle,GL_LINK_STATUS,&linkStatus);  
	if(GL_FALSE == linkStatus)  
	{  
		printf("ERROR : link shader program failed");  
		GLint logLen;  
		glGetProgramiv(programHandle,GL_INFO_LOG_LENGTH, &logLen);  
		if (logLen > 0)  
		{  
			char *log = (char *)malloc(logLen);  
			GLsizei written;  
			glGetProgramInfoLog(programHandle,logLen, &written,log);  
			printf("Program log :%s\n", log);  
		}  
	}  
	else glUseProgram(programHandle);  
}
void initshape()
{
	void* vertexdata  = buffer+0x000000;
	void* normaldata  = buffer+0x100000;
	void* colordata   = buffer+0x200000;
	void* texturedata = buffer+0x300000;
	void* pointindex  = buffer+0x400000;
	void* lineindex   = buffer+0x500000;
	void* triindex    = buffer+0x600000;
	void* rectindex   = buffer+0x700000;

	//
	glGenBuffers(1, &vertexhandle);
	glBindBuffer(GL_ARRAY_BUFFER, vertexhandle);
	glBufferData(GL_ARRAY_BUFFER, 0x100000, vertexdata, GL_STATIC_DRAW);

	glGenBuffers(1, &normalhandle);
	glBindBuffer(GL_ARRAY_BUFFER, normalhandle);
	glBufferData(GL_ARRAY_BUFFER, 0x100000, normaldata, GL_STATIC_DRAW);

	glGenBuffers(1, &colourhandle);
	glBindBuffer(GL_ARRAY_BUFFER, colourhandle);
	glBufferData(GL_ARRAY_BUFFER, 0x100000, colordata, GL_STATIC_DRAW);

	glGenBuffers(1, &texturehandle);
	glBindBuffer(GL_ARRAY_BUFFER, texturehandle);
	glBufferData(GL_ARRAY_BUFFER, 0x100000, colordata, GL_STATIC_DRAW);


	//point
	glGenVertexArrays(1, &shapevao1);
	glBindVertexArray(shapevao1);
	glBindBuffer(GL_ARRAY_BUFFER, vertexhandle);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, normalhandle);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colourhandle);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glGenBuffers(1, &indexhandle1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexhandle1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0x100000, pointindex, GL_STATIC_DRAW);


	//line
	glGenVertexArrays(1, &shapevao2);
	glBindVertexArray(shapevao2);
	glBindBuffer(GL_ARRAY_BUFFER, vertexhandle);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, normalhandle);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colourhandle);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glGenBuffers(1, &indexhandle2);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexhandle2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0x100000, lineindex, GL_STATIC_DRAW);


	//tria
	glGenVertexArrays(1, &shapevao3);
	glBindVertexArray(shapevao3);
	glBindBuffer(GL_ARRAY_BUFFER, vertexhandle);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, normalhandle);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colourhandle);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, texturehandle);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

	glGenBuffers(1, &indexhandle3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexhandle3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0x100000, triindex, GL_STATIC_DRAW);

/*
	//rect
	glGenVertexArrays(1, &shapevao4);
	glBindVertexArray(shapevao4);
	glBindBuffer(GL_ARRAY_BUFFER, vertexhandle);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, normalhandle);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colourhandle);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, texturehandle);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

	glGenBuffers(1, &indexhandle4);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexhandle4);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0x100000, rectindex, GL_STATIC_DRAW);
*/
}
void graph_data(void* buf, void* ind, void* cb, int cl)
{
	ctxbuf = cb;
	ctxlen = cl;
	enqueue = (enqueue+1)%0x10000;
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
	GLfloat temp[4*4];

	fixmodel();
	fixview();
	fixprojection();

	for(x=0;x<16;x++)temp[x] = modelmatrix[x];
	matrixmultiply(temp, viewmatrix);
	matrixmultiply(temp, projmatrix);

	GLint pvmmatrix = glGetUniformLocation(programHandle, "modelviewproj");
	glUniformMatrix4fv(pvmmatrix, 1, GL_FALSE, temp);
}
void fixlight()
{
	GLfloat ambientcolor[3] = {0.5f, 0.5f, 0.5f};
	GLfloat lightcolor[3] = {1.0f, 1.0f, 1.0f};
	GLfloat lightposition[3] = {0.0f, 0.0f, 10.0f};

	GLint ac = glGetUniformLocation(programHandle, "ambientcolor");
	glUniform3fv(ac, 1, ambientcolor);

	GLint dc = glGetUniformLocation(programHandle, "lightcolor");
	glUniform3fv(dc, 1, lightcolor);

	GLint dp = glGetUniformLocation(programHandle, "lightposition");
	glUniform3fv(dp, 1, lightposition);

	GLint ep = glGetUniformLocation(programHandle, "lightposition");
	glUniform3fv(ep, 1, camera);
}
void callback_display()
{
	int j, type;

	//set
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	//
	fixmatrix();
	fixlight();

	//glBindVertexArray(shapevao4);
	//glDrawElements(GL_QUADS, binfo->rectcount, GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(shapevao3);
	glDrawElements(GL_TRIANGLES, 3*binfo->tricount, GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(shapevao2);
	glDrawElements(GL_LINES, 2*binfo->linecount, GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(shapevao1);
	glDrawElements(GL_POINTS, binfo->pointcount, GL_UNSIGNED_SHORT, 0);

	//write
	glFlush();
	glutSwapBuffers();
}
void callback_idle()
{
	float* vertexdata;
	float* normaldata;
	float* colourdata;
	float* texturedata;
	u16* pointindex;
	u16* lineindex;
	u16* triindex;
	u16* rectindex;

//printf("enq=%d,deq=%d\n", enqueue, dequeue);
	if(enqueue != dequeue)
	{
		dequeue = (dequeue+1)%0x10000;
	}
	else
	{
//printf("haha\n");
		graph_hack(buffer, binfo, ctxbuf, ctxlen);
	}

	vertexdata  = buffer + 0x000000;
	normaldata  = buffer + 0x100000;
	colourdata  = buffer + 0x200000;
	texturedata = buffer + 0x300000;
	pointindex  = buffer + 0x400000;
	lineindex   = buffer + 0x500000;
	triindex    = buffer + 0x600000;
	rectindex   = buffer + 0x700000;

	if(binfo->vertexcount != 0)
	{
//printf("vertexcount=%d\n",binfo->vertexcount);
		glBindBuffer(GL_ARRAY_BUFFER, vertexhandle);
		glBufferSubData(GL_ARRAY_BUFFER, 0,
			12 * binfo->vertexcount, vertexdata);

		glBindBuffer(GL_ARRAY_BUFFER, normalhandle);
		glBufferSubData(GL_ARRAY_BUFFER, 0,
			12 * binfo->vertexcount, normaldata);

		glBindBuffer(GL_ARRAY_BUFFER, colourhandle);
		glBufferSubData(GL_ARRAY_BUFFER, 0,
			12 * binfo->vertexcount, colourdata);
	}
	if(binfo->pointcount != 0)
	{
//printf("pointcount=%d\n",binfo->pointcount);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexhandle1);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
			2*binfo->pointcount, pointindex);
	}
	if(binfo->linecount != 0)
	{
//printf("linecount=%d\n",binfo->linecount);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexhandle2);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
			4*binfo->linecount, lineindex);
	}
	if(binfo->tricount != 0)
	{
//printf("tricount=%d\n",binfo->tricount);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexhandle3);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
			6*binfo->tricount, triindex);
	}
/*
	if(binfo->rectcount != 0)
	{
//printf("rectcount=%d\n",binfo->rectcount);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexhandle4);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0,
			8*binfo->rectcount, rectindex);
	}
*/
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
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB + GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutInitWindowPosition(256, 256);
	glutCreateWindow("evil");
	glEnable(GL_DEPTH_TEST);

	err = glewInit();
	if( GLEW_OK != err )printf("glewinit: %s\n", glewGetErrorString(err));  

	glPointSize(2.0);
	glViewport(0, 0, 512, 512);
	glEnable(GL_DEPTH_TEST);
	initshader();
	initshape();

	glutIdleFunc(callback_idle);
	glutDisplayFunc(callback_display);
	glutKeyboardFunc(callback_keyboard);
	glutReshapeFunc(callback_reshape);
	glutMouseFunc(callback_mouse);
	glutMotionFunc(callback_move); 
	  
	glutMainLoop();

	glDeleteShader(vShader);
	glDeleteShader(fShader);
	glUseProgram(0);
	return 0;
}
void graph_init(void* buf, void* ind, void* cb, int cl)
{
	buffer = buf;
	binfo = ind;
	ctxbuf = cb;
	ctxlen = cl;

	u64 id;
	pthread_create((void*)&id, NULL, graph_thread, 0);
}
