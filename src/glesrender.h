
// We're trying this define later.
// or removing it permanently
//#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

static struct
{
	GLuint windowBase;
}*surfaceBuffers;
	
void
initBuffers()
{
	surfaceBuffers = malloc(sizeof(*surfaceBuffers));

	glGenBuffers(1,&surfaceBuffers->windowBase);

	// [x][y]
	float windowBody[] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};
	glBindBuffer(GL_ARRAY_BUFFER, surfaceBuffers->windowBase);
	glBufferData(GL_ARRAY_BUFFER, sizeof(windowBody),
			windowBody, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 
			sizeof(float)*2,0);

	GLuint desktopShader_vertex = 
		glCreateShader(GL_VERTEX_SHADER);

	GLuint desktopShader_fragment =
		glCreateShader(GL_FRAGMENT_SHADER);

	if(GL_NUM_SHADER_BINARY_FORMATS > 0)
		printf( "Loading from bina"
			"ry for shaders ar"
			"e are allowed.\n");
	/*	
	glShaderSource(desktopShader_vertex,
			)
	*/

	glCompileShader(desktopShader_vertex);
}

void
destroyBuffers()
{
	glDeleteBuffers(1,&surfaceBuffers->windowBase);
	free(surfaceBuffers);
}

void
drawWindowBuffer()
{
	glBindBuffer(GL_ARRAY_BUFFER, surfaceBuffers->windowBase);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


// Actual Rendering

void render_surface(struct Surface* surface)
{
	glClearColor(0.2, 0.2, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
}
