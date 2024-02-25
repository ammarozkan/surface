
// We're trying this define later.
// or removing it permanently
//#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "gltools.h"

static struct
{
	GLuint rectangle;
	GLuint vCursor;
}*surfaceBuffers;

static struct
{
	GLuint dWindowVertex;
	GLuint dWindowFragment;
	GLuint dWindowProgram;
	GLint d_w_s;
	GLint d_w_e;

	GLuint dCursorVertex;
	GLuint dCursorFragment;
	GLuint dCursorProgram;
	GLint CursorProgram_cursorPos;

}*surfacePrograms;

void
initBuffers()
{
	// [x][y]
	float rectangle[] = {
		0.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 0.0f,
		1.0f, 1.0f
	};

	float vCursor[] = {
		1.0f, 0.0f,
		0.0f, 0.0f,
		0.4f, 0.4f,
		0.0f, 1.0f
	};
	
	surfaceBuffers = malloc(sizeof(*surfaceBuffers));

	surfaceBuffers->rectangle = create2dBuffer(rectangle, sizeof(rectangle));
	//surfaceBuffers->vCursor = create2dBuffer(vCursor, sizeof(vCursor));

	glClearColor(0.2, 0.2, 0.2, 1.0);
}

void
destroyBuffers()
{
	glDeleteBuffers(1,&surfaceBuffers->rectangle);
	free(surfaceBuffers);
}

int
initWindowProgram()
{
	surfacePrograms->dWindowVertex = 
		getShaderFromFile("shaders/dWindowVertex.glsl", 
				GL_VERTEX_SHADER);
	
	if(!surfacePrograms->dWindowVertex) {
		printf("dWindowVertex shader couldn't be created.\n");
		return 0;
	}

	surfacePrograms->dWindowFragment =
		getShaderFromFile("shaders/dWindowFragment.glsl", 
				GL_FRAGMENT_SHADER);
	
	if(!surfacePrograms->dWindowFragment) {
		printf("dWindowFragment shader couldn't be created.\n");
		return 0;
	}
	
	surfacePrograms->dWindowProgram = 
		getProgram(surfacePrograms->dWindowVertex,
				surfacePrograms->dWindowFragment);
	glBindAttribLocation(surfacePrograms->dWindowProgram, 0, "position");

	if(!linkProgram(surfacePrograms->dWindowProgram)) return 0;

	surfacePrograms->d_w_s = glGetUniformLocation(
			surfacePrograms->dWindowProgram, "w_s");
	surfacePrograms->d_w_e = glGetUniformLocation(
			surfacePrograms->dWindowProgram, "w_e");
	return 1;
}

int
initCursorProgram()
{
	surfacePrograms->dCursorVertex = 
		getShaderFromFile("shaders/dCursorVertex.glsl",
				GL_VERTEX_SHADER);
	if(!surfacePrograms->dCursorVertex) {
		printf("dCursorVertex shader couldn't be created.\n");
		return 0;
	}

	surfacePrograms->dCursorFragment =
		getShaderFromFile("shaders/dCursorFragment.glsl",
				GL_FRAGMENT_SHADER);

	if(!surfacePrograms->dCursorFragment) {
		printf("dCursorFragment shader couldn't be created.\n");
		return 0;
	}

	surfacePrograms->dCursorProgram =
		getProgram(surfacePrograms->dCursorVertex,
				surfacePrograms->dCursorFragment);
	glBindAttribLocation(surfacePrograms->dCursorProgram, 0, "model_pos");
	
	if(!linkProgram(surfacePrograms->dCursorProgram)) return 0;

	surfacePrograms->CursorProgram_cursorPos =
		glGetUniformLocation(surfacePrograms->dCursorProgram, 
				"cursorPos");
	if(!surfacePrograms->CursorProgram_cursorPos) {
		printf("cursorPos uniform not found in cursor program.\n");
		return 0;
	}
	return 1;
}

int
initPrograms()
{
	GLint ivRes;
	char* loadedFile;

	surfacePrograms = malloc(sizeof(*surfacePrograms));

	if(GL_NUM_SHADER_BINARY_FORMATS > 0)
		printf( "Loading from bina"
			"ry for shaders ar"
			"e are allowed.\n");
	
	if(GL_SHADER_COMPILER != GL_TRUE)
		printf( "Shader compiler i"
			"s not supported. "
			"Please contact so"
			"me support.\n");

	if(!initWindowProgram()) {
		return 0;
	} else if(!initCursorProgram()) {
		return 0;
	}

	glReleaseShaderCompiler();

	return 1;
}

void
destroyPrograms()
{
	glDeleteProgram(surfacePrograms->dWindowProgram);

	glDeleteShader(surfacePrograms->dWindowVertex);
	glDeleteShader(surfacePrograms->dWindowFragment);
}


void
initProgramScreenSize(uint32_t width, uint32_t height)
{
	GLuint wsize_uniform;
	glUseProgram(surfacePrograms->dWindowProgram);
	wsize_uniform = glGetUniformLocation(
			surfacePrograms->dWindowProgram, "screen_size");
	
	if(wsize_uniform == 0) {
		printf("Uniform %s NOT FOUND!\n", "screen_size");
		return;
	}
	
	glUniform2f(wsize_uniform, (float)width, (float)height);
	
	GLuint csize_uniform;
	glUseProgram(surfacePrograms->dCursorProgram);
	csize_uniform = glGetUniformLocation(
			surfacePrograms->dCursorProgram, "screen_size");
	
	glUniform2f(csize_uniform, (float)width, (float)height);
}

void
drawCursorBuffer(struct QuickCursor qc)
{
	float posx = qc.x, posy = qc.y;
	glUseProgram(surfacePrograms->dCursorProgram);
	glUniform2f(surfacePrograms->CursorProgram_cursorPos, posx, posy);

	glBindBuffer(GL_ARRAY_BUFFER, surfaceBuffers->vCursor);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void
drawWindowBuffer(float sx, float sy, float ex, float ey)
{
	glUseProgram(surfacePrograms->dWindowProgram);
	glUniform2f(surfacePrograms->d_w_s, sx, sy);
	glUniform2f(surfacePrograms->d_w_e, ex, ey);

	glBindBuffer(GL_ARRAY_BUFFER, surfaceBuffers->rectangle);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


// Actual Rendering

void render_surface(struct Surface* surf)
{
	glClear(GL_COLOR_BUFFER_BIT);
	
	SURF_ITERATE(struct Window, surf->wins, win)
	{
		drawWindowBuffer(win->sx, win->sy,
				win->ex, win->ey);
	}
//	drawWindowBuffer(100.0f, 100.0f, 300.0f, 200.0f);
	drawCursorBuffer(surf->cursor);

}
