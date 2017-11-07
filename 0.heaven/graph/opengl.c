#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
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
float camerax = 1.0f;
float cameray = -2.0f;
float cameraz = 1.0f;
float centerx = 0.0f;
float centery = 0.0f;
float centerz = 0.0f;
float abovex = 0.0f;
float abovey = 0.0f;
float abovez = 1.0f;
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
int shapecount[5];
GLuint vao[5];
GLuint positionhandle[5];
GLuint normalhandle[5];
GLuint colorhandle[5];




char vCode[] = {
	"#version 400\n"
	"layout(location = 0)in vec3 position;\n"
	"layout(location = 1)in vec3 normal;\n"
	"layout(location = 2)in vec3 color;\n"
	"out vec3 vertexcolor;\n"
	"uniform vec3 ambientcolor;\n"
	"uniform vec3 diffusecolor;\n"
	"uniform vec3 diffuseplace;\n"
	"uniform mat4 mvpmatrix;\n"
	//"uniform mat4 normatrix;\n"
	"void main()\n"
	"{\n"
		"vec3 N = normalize(normal);"
		"vec3 S = normalize(vec3(diffuseplace - position));"
		"vec3 ddd = diffusecolor * max(dot(S, N), 0.0);\n"
		"vertexcolor = color + ambientcolor + ddd;\n"
		"gl_Position = mvpmatrix * vec4(position,1.0);\n"
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
    printf("GL Vendor: %s\n", vendor);
    printf("GL Renderer: %s\n", renderer);
    printf("GL Version (string): %s\n", version);
    printf("GLSL Version: %s\n", glslVersion);
    printf("GL Version (integer): %x.%x\n", major, minor);

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
	int j,k;
	//pointdata_start(1);
	pointindex_start(1);
	//shapedata_start(1);
	shapeindex_start(1);
	strdata_start(1);
	strhash_start(1);
	connect_start(1);

	void* positiondata = malloc(0x100000);
	void* normaldata = malloc(0x100000);
	void* colordata = malloc(0x100000);

	for(j=1;j<5;j++)
    {
		if(j == 1)k = mesh_point(positiondata, normaldata, colordata);
		else if(j == 2)k = mesh_line(positiondata, normaldata, colordata);
		else if(j == 3)k = mesh_tri(positiondata, normaldata, colordata);
		else if(j == 4)k = mesh_rect(positiondata, normaldata, colordata);
		if(k == 0)continue;
		shapecount[j] = j*k;

		glGenVertexArrays(1, &vao[j]);
		glBindVertexArray(vao[j]);

		glGenBuffers(1, &positionhandle[j]);
		glBindBuffer(GL_ARRAY_BUFFER, positionhandle[j]);
		glBufferData(GL_ARRAY_BUFFER, 12 * shapecount[j], positiondata, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &normalhandle[j]);
		glBindBuffer(GL_ARRAY_BUFFER, normalhandle[j]);
		glBufferData(GL_ARRAY_BUFFER, 12 * shapecount[j], normaldata, GL_STATIC_DRAW);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);

		glGenBuffers(1, &colorhandle[j]);
		glBindBuffer(GL_ARRAY_BUFFER, colorhandle[j]);
		glBufferData(GL_ARRAY_BUFFER, 12 * shapecount[j], colordata, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(2);
	}
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
	float nx = centerx - camerax;
	float ny = centery - cameray;
	float nz = centerz - cameraz;
	norm = sqrt(nx*nx + ny*ny + nz*nz);
	nx /= norm;
	ny /= norm;
	nz /= norm;

	//X = cross(Z, above)
	float ux = ny*abovez - nz*abovey;
	float uy = nz*abovex - nx*abovez;
	float uz = nx*abovey - ny*abovex;
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

	viewmatrix[12] = -camerax*ux - cameray*uy - cameraz*uz;
	viewmatrix[13] = -camerax*vx - cameray*vy - cameraz*vz;
	viewmatrix[14] = camerax*nx + cameray*ny + cameraz*nz;
	viewmatrix[15] = 1.0f;
/*
	viewmatrix[0] = cos(camerax);
	viewmatrix[2] = -sin(camerax);
	viewmatrix[8] = sin(camerax);
	viewmatrix[10] = cos(camerax);
	viewmatrix[14] = -1.0f;
*/
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

	GLint pvmmatrix = glGetUniformLocation(programHandle, "mvpmatrix");
	glUniformMatrix4fv(pvmmatrix, 1, GL_FALSE, temp);
}
void fixlight()
{
	GLfloat ambientcolor[3] = {0.1f, 0.1f, 0.1f};
	GLfloat diffuseplace[3] = {0.1f, 0.2f, 5.0f};
	GLfloat diffusecolor[3] = {0.5f, 0.5f, 0.5f};

	GLint ac = glGetUniformLocation(programHandle, "ambientcolor");
	glUniform3fv(ac, 1, ambientcolor);

	GLint dc = glGetUniformLocation(programHandle, "diffusecolor");
	glUniform3fv(dc, 1, diffusecolor);

	GLint dp = glGetUniformLocation(programHandle, "diffuseplace");
	glUniform3fv(dp, 1, diffuseplace);
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

	for(j=1;j<=4;j++)
	{
		if(j == 1)type = GL_POINTS;
		else if(j == 2)type = GL_LINES;
		else if(j == 3)type = GL_TRIANGLES;
		else if(j == 4)type = GL_QUADS;
		glBindVertexArray(vao[j]);
		glDrawArrays(type, 0, shapecount[j]);
	}

	//write
	glFlush();
    glutSwapBuffers();
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
	float tx = camerax;
	float ty = cameray;
	if(x>last_x)
	{
		camerax = tx*cos(0.1f) + ty*sin(0.1f);
		cameray = -tx*sin(0.1f) + ty*cos(0.1f);

		//camera_yaw += PI/20;
	}
	else if(x<last_x)
	{
		camerax = tx*cos(0.1f) - ty*sin(0.1f);
		cameray = tx*sin(0.1f) + ty*cos(0.1f);

		//camera_yaw -= PI/20;
	}

	if(y>last_y)
	{
		cameraz += 0.1;
		//cameray += 0.1f;
		//if(camera_pitch < PI*44/90)camera_pitch += PI/90;
	}
	else if(y<last_y)
	{
		cameraz -= 0.1;
		//cameray -= 0.1f;
		//if(camera_pitch > -PI*44/90)camera_pitch -= PI/90;
	}

	last_x = x;
	last_y = y;
	glutPostRedisplay();
}
void callback_mouse(int button, int state, int x, int y)
{
	float tx = camerax;
	float ty = cameray;
	float tz = cameraz;
	if(state == GLUT_DOWN)
	{
		last_x = x;
		last_y = y;
	}
	if(state == GLUT_UP)
	{
		if(button == 3)	//wheel_up
		{
			camerax = 0.9*tx + 0.1*centerx;
			cameray = 0.9*ty + 0.1*centery;
			cameraz = 0.9*tz + 0.1*centerz;
/*
			//camera_zoom *= 0.95;
			viewmatrix[14] += 0.1f;
*/
			glutPostRedisplay();
		}
		if(button == 4)	//wheel_down
		{
			camerax = 1.1*tx - 0.1*centerx;
			cameray = 1.1*ty - 0.1*centery;
			cameraz = 1.1*tz - 0.1*centerz;
			glutPostRedisplay();
/*
			viewmatrix[14] -= 0.1f;
			//camera_zoom *= 1.05263158;
*/
		}
		//printf("camera_zoom=%f\n",camera_zoom);
	}
}
int graph(int argc,char** argv)  
{
	int err;
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(512, 512);
    glutInitWindowPosition(256, 256);
    glutCreateWindow("GLSL Test : Draw a triangle");

    err = glewInit();
    if( GLEW_OK != err )printf("glewinit: %s\n", glewGetErrorString(err));  

	glViewport(0, 0, 512, 512);
	glEnable(GL_DEPTH_TEST);
    initshader();
    initshape();

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