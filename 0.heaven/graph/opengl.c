#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#define u8 unsigned char
#define u16 unsigned short
#define u32 unsigned int
#define u64 unsigned long long
#define PI 3.1415926535897932384626433832795028841971693993151
//
int width;
int height;
int last_x=0;
int last_y=0;
//
GLuint vShader;
GLuint fShader;
GLuint programHandle;
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
GLuint vertexhandle;
GLuint normalhandle;
GLuint colourhandle;
GLuint texturehandle;
//
GLuint shapevao4;
GLuint indexhandle4;
GLuint shapevao3;
GLuint indexhandle3;
GLuint shapevao2;
GLuint indexhandle2;
GLuint shapevao1;
GLuint indexhandle1;
//
u8* buffer = 0;
u64* binfo = 0;
int enqueue = 0;
int dequeue = 0;




char vCode[] = {
	"#version 400\n"
	"layout(location = 0)in vec3 position;\n"
	"layout(location = 1)in vec3 normal;\n"
	"layout(location = 2)in vec3 color;\n"
	"uniform vec3 ambientcolor;\n"
	"uniform vec3 lightcolor;\n"
	"uniform vec3 lightposition;\n"
	"uniform vec3 eyeposition;\n"
	"uniform mat4 modelviewproj;\n"
	"uniform mat4 normalmatrix;\n"
	"out vec3 vertexcolor;\n"
	"void main()\n"
	"{\n"
		"vec3 N = normalize(normal);\n"
		"vec3 L = normalize(vec3(lightposition - position));\n"
		"vec3 E = normalize(eyeposition-position);\n"
		"vec3 R = reflect(-L, N);\n"
		"float SN = max(dot(N, L), 0.0);\n"
		"float RV = max(dot(R, E), 0.0);\n"
		"vec3 ambient = color * ambientcolor;\n"
		"vec3 diffuse = color * lightcolor * SN;\n"
		"vec3 specular = vec3(0.0, 0.0, 0.0);\n"
		"if(SN>0.0)specular = color * lightcolor * pow(RV, 8);\n"
		"vertexcolor = ambient + diffuse + specular;\n"
		"gl_Position = modelviewproj * vec4(position,1.0);\n"
	"}\n"
};
char fCode[] = {
	"#version 400\n"
	"in vec3 vertexcolor;\n"
	"out vec4 FragColor;\n"
	"void main()\n"
	"{\n"
		"FragColor = vec4(vertexcolor,1.0);\n"
	"}\n"
};
void initshader()  
{  
    //1. 查看GLSL和OpenGL的版本  
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

    //2. 顶点着色器  
    vShader = glCreateShader(GL_VERTEX_SHADER);
    if (0 == vShader)  
    {  
        printf("ERROR : Create vertex shader failed\n");
        exit(1);  
    }  

    //把着色器源代码和着色器对象相关联
	const GLchar* vCodeArray[1] = {vCode};
    glShaderSource(vShader, 1, vCodeArray, NULL);
    glCompileShader(vShader);  

    //检查编译是否成功  
    GLint compileResult;  
    glGetShaderiv(vShader,GL_COMPILE_STATUS,&compileResult);  
    if (GL_FALSE == compileResult)  
    {  
        GLint logLen;  
        //得到编译日志长度  
        glGetShaderiv(vShader,GL_INFO_LOG_LENGTH,&logLen);  
        if (logLen > 0)  
        {  
            char *log = (char *)malloc(logLen);  
            GLsizei written;  
            //得到日志信息并输出  
            glGetShaderInfoLog(vShader,logLen,&written,log);
            printf("vertex shader compile log: %s\n",log);
            free(log);//释放空间
        }
    }

    //3. 片断着色器  
    fShader = glCreateShader(GL_FRAGMENT_SHADER);
    if (0 == fShader)  
    {  
        printf("ERROR : Create fragment shader failed");  
        exit(1);  
    }  

    //把着色器源代码和着色器对象相关联
	const GLchar* fCodeArray[1] = {fCode};
    glShaderSource(fShader, 1, fCodeArray, NULL);
    glCompileShader(fShader);  

    //检查编译是否成功  
    glGetShaderiv(fShader,GL_COMPILE_STATUS,&compileResult);  
    if (GL_FALSE == compileResult)  
    {  
        GLint logLen;  
        //得到编译日志长度  
        glGetShaderiv(fShader,GL_INFO_LOG_LENGTH,&logLen);  
        if (logLen > 0)  
        {  
            char *log = (char *)malloc(logLen);  
            GLsizei written;  
            //得到日志信息并输出  
            glGetShaderInfoLog(fShader,logLen,&written,log);
            printf("fragment shader compile log: %s\n",log);
            free(log);//释放空间  
        }  
    }  
  
    //4. 着色器程序  
    programHandle = glCreateProgram();  
    if (!programHandle)  
    {  
        printf("ERROR : create program failed");
        exit(1);  
    }

    //将着色器程序链接到所创建的程序中  
    glAttachShader(programHandle,vShader);
    glAttachShader(programHandle,fShader);
    glLinkProgram(programHandle);

    //查询链接的结果  
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
    else//链接成功，在OpenGL管线中使用渲染程序  
    {  
        glUseProgram(programHandle);  
    }  
}
void initshape()
{
	void* vertexxyz = (void*)buffer+0x000000;
	void* normalxyz = (void*)buffer+0x100000;
	void* colorrgb = (void*)buffer+0x200000;
	void* texturexyz = (void*)buffer+0x300000;

	void* pointindex = (void*)buffer+0x400000;
	void* lineindex = (void*)buffer+0x500000;
	void* triindex = (void*)buffer+0x600000;
	void* rectindex = (void*)buffer+0x700000;

	//
	glGenBuffers(1, &vertexhandle);
	glBindBuffer(GL_ARRAY_BUFFER, vertexhandle);
    glBufferData(GL_ARRAY_BUFFER, 0x100000, vertexxyz, GL_STATIC_DRAW);

	glGenBuffers(1, &normalhandle);
	glBindBuffer(GL_ARRAY_BUFFER, normalhandle);
    glBufferData(GL_ARRAY_BUFFER, 0x100000, normalxyz, GL_STATIC_DRAW);

	glGenBuffers(1, &colourhandle);
	glBindBuffer(GL_ARRAY_BUFFER, colourhandle);
    glBufferData(GL_ARRAY_BUFFER, 0x100000, colorrgb, GL_STATIC_DRAW);


	//
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

	glGenBuffers(1, &indexhandle4);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexhandle4);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0x100000, rectindex, GL_STATIC_DRAW);


	//
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

	glGenBuffers(1, &indexhandle3);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexhandle3);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 0x100000, triindex, GL_STATIC_DRAW);


	//
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


	//
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
}
void graph_data(
	void* vbuf, int vlen,
	void* nbuf, int nlen,
	void* cbuf, int clen,
	u16* rbuf, int rlen,
	u16* tbuf, int tlen,
	u16* lbuf, int llen,
	u16* pbuf, int plen)
{
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

	glBindVertexArray(shapevao4);
	glDrawElements(GL_QUADS, binfo[4], GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(shapevao3);
	glDrawElements(GL_TRIANGLES, binfo[5], GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(shapevao2);
	glDrawElements(GL_LINES, binfo[6], GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(shapevao1);
	glDrawElements(GL_POINTS, binfo[7], GL_UNSIGNED_SHORT, 0);

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
	if(enqueue == dequeue)return;

	vertexdata = (void*)buffer + 0x000000;
	normaldata = (void*)buffer + 0x100000;
	colourdata = (void*)buffer + 0x200000;
	texturedata = (void*)buffer + 0x300000;

	rectindex = (void*)buffer + 0x400000;
	triindex = (void*)buffer + 0x500000;
	lineindex = (void*)buffer + 0x600000;
	pointindex = (void*)buffer + 0x700000;

	if(binfo[0] != 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, vertexhandle);
		glBufferSubData(GL_ARRAY_BUFFER, 0, 12 * binfo[0], vertexdata);

		glBindBuffer(GL_ARRAY_BUFFER, normalhandle);
		glBufferSubData(GL_ARRAY_BUFFER, 0, 12 * binfo[0], normaldata);

		glBindBuffer(GL_ARRAY_BUFFER, colourhandle);
		glBufferSubData(GL_ARRAY_BUFFER, 0, 12 * binfo[0], colourdata);
	}
	if(binfo[4] != 0)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexhandle4);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 2*binfo[4], rectindex);
	}
	if(binfo[5] != 0)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexhandle3);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 2*binfo[5], triindex);
	}
	if(binfo[6] != 0)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexhandle2);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 2*binfo[6], lineindex);
	}
	if(binfo[7] != 0)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexhandle1);
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, 2*binfo[7], pointindex);
	}

	dequeue = (dequeue+1)%0x10000;
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
void graph_thread()
{
	int err;
	int argc = 1;
	char* argv[2] = {"a.out", 0};
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(512, 512);
    glutInitWindowPosition(256, 256);
    glutCreateWindow("GLSL Test : Draw a triangle");

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
}
void graph_init(void* buf, void* ind)
{
	buffer = buf;
	binfo = ind;

	u64 id;
	pthread_create((void*)&id, NULL, graph_thread, 0);
}