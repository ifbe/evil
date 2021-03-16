#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>




void render_init(void* cb, int cl, void* lb, int ll)
{
}
void render_free()
{
}
void render_data(void* cb, int cl, void* lb, int ll)
{
	printf("nodecnt=%d,wirecnt=%d\n", cl, ll);
}
