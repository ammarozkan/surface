
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
	GLuint windowVertex;
	GLuint windowFragment;
	GLuint windowProgram;
	GLint d_w_s;
	GLint d_w_e;

	GLuint cursorVertex;
	GLuint cursorFragment;
	GLuint cursorProgram;
	GLint CursorProgram_cursorPos;
	GLint CursorProgram_screen_size;

	GLuint backgroundVertex;
	GLuint backgroundFragment;
	GLuint backgroundProgram;

	GLuint menubarVertex;
	GLuint menubarFragment;
	GLuint menubarProgram;

}*surfacePrograms;

int
initBuffers()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

	surfaceBuffers->rectangle = create2dBuffer(rectangle, 
			sizeof(rectangle));
	//surfaceBuffers->vCursor = create2dBuffer(vCursor, sizeof(vCursor));

	glClearColor(0.2, 0.2, 0.2, 1.0);
	return 1;
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
	int* ids = initClassicalProgram(
			"shaders/windowVertex.glsl",
			"shaders/windowFragment.glsl");
	if(ids == NULL) {
		printf("window pogram cannot be inited.\n");
	}
	glBindAttribLocation(ids[2], 0, "model_pos");

	surfacePrograms->d_w_s = glGetUniformLocation(ids[2], "w_s");
	surfacePrograms->d_w_e = glGetUniformLocation(ids[2], "w_e");

	if(!linkProgram(ids[2])) return 0;

	surfacePrograms->windowVertex = ids[0];
	surfacePrograms->windowFragment = ids[1];
	surfacePrograms->windowProgram = ids[2];
	free(ids);
	return 1;

errexit:
	free(ids);
	return 0;
}

int
initCursorProgram()
{	
	int* ids = initClassicalProgram(
			"shaders/cursorVertex.glsl",
			"shaders/cursorFragment.glsl");
	if(ids == NULL) {
		printf("cursor pogram cannot be inited.\n");
	}
	glBindAttribLocation(ids[2], 0, "model_pos");

	surfacePrograms->CursorProgram_cursorPos = glGetUniformLocation(ids[2], "cursorPos");
	if(surfacePrograms->CursorProgram_cursorPos == 0) {
		printf("cursorPos uniform not found in cursor program.\n");
		return 0;
	}

	surfacePrograms->CursorProgram_screen_size = glGetUniformLocation(ids[2], "screen_size");
	if(surfacePrograms->CursorProgram_cursorPos == 0) {
		printf("screen_size uniform not found in cursor program.\n");
		return 0;
	}
	
	if(!linkProgram(ids[2])) return 0;


	surfacePrograms->cursorVertex = ids[0];
	surfacePrograms->cursorFragment = ids[1];
	surfacePrograms->cursorProgram = ids[2];
	free(ids);
	return 1;

errexit:
	free(ids);
	return 0;
}

int
initBackgroundProgram()
{
	int* ids = initClassicalProgram(
			"shaders/backgroundVertex.glsl",
			"shaders/backgroundFragment.glsl");
	if(ids == NULL) {
		printf("bckr pogram cannot be inited.\n");
		goto errexit;
	}
	glBindAttribLocation(ids[2], 0, "model_pos");

	if(!linkProgram(ids[2])) return 0;

	surfacePrograms->backgroundVertex = ids[0];
	surfacePrograms->backgroundFragment = ids[1];
	surfacePrograms->backgroundProgram = ids[2];
	free(ids);
	return 1;

errexit:
	free(ids);
	return 0;
}

int
initMenubarProgram()
{
	int* ids = initClassicalProgram(
			"shaders/menubarVertex.glsl",
			"shaders/menubarFragment.glsl");
	if(ids == NULL) {
		printf("menu program cannot be inited.\n");
		return 0;
	}
	glBindAttribLocation(ids[2], 0, "model_pas");
	if(!linkProgram(ids[2])) goto erralloc;
		
	surfacePrograms->menubarVertex = ids[0];
	surfacePrograms->menubarFragment = ids[1];
	surfacePrograms->menubarProgram = ids[2];
	free(ids);
	return 1;
erralloc:
	free(ids);
errexit:
	return 0;
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
	} else if(!initBackgroundProgram()) {
		return 0;
	} else if(!initMenubarProgram()) {
		return 0;
	}

	glReleaseShaderCompiler();

	return 1;
}

void
destroyPrograms()
{
	glDeleteProgram(surfacePrograms->windowProgram);

	glDeleteShader(surfacePrograms->windowVertex);
	glDeleteShader(surfacePrograms->windowFragment);
}


void
initProgramScreenSize(uint32_t width, uint32_t height)
{
	GLuint wsize_uniform;
	glUseProgram(surfacePrograms->windowProgram);
	wsize_uniform = glGetUniformLocation(
			surfacePrograms->windowProgram, "screen_size");
	
	glUniform2f(wsize_uniform, (float)width, (float)height);
	
	if(wsize_uniform == -1) {
		printf("Uniform %s NOT FOUND in window!\n", "screen_size");
	}

	glUseProgram(surfacePrograms->menubarProgram);
	wsize_uniform = glGetUniformLocation(
			surfacePrograms->menubarProgram, "screen_size");

	glUniform2f(wsize_uniform, (float)width, (float)height);
	
	if(wsize_uniform == -1) {
		printf("Uniform %s NOT FOUND in menubar!\n", "screen_size");
	}
	
	glUseProgram(surfacePrograms->cursorProgram);
	wsize_uniform = glGetUniformLocation(
			surfacePrograms->cursorProgram, "screen_size");
	
	glUniform2f(wsize_uniform, (float)width, (float)height);
	
	if(wsize_uniform == -1) {
		printf("Uniform %s NOT FOUND in london!\n", "screen_size");
	}

	return;
errret:
	printf("Uniform %s NOT FOUND!\n", "screen_size");
	return;
}

void
drawMenuBar()
{
	glUseProgram(surfacePrograms->menubarProgram);	
	
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void
drawCursorBuffer(struct QuickCursor qc)
{
	float posx = qc.x, posy = qc.y;
	glUseProgram(surfacePrograms->cursorProgram);
	glUniform2f(surfacePrograms->CursorProgram_cursorPos, posx, posy);

	glBindBuffer(GL_ARRAY_BUFFER, surfaceBuffers->vCursor);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void
drawWindowBuffer(float sx, float sy, float ex, float ey)
{
	glUseProgram(surfacePrograms->windowProgram);
	glUniform2f(surfacePrograms->d_w_s, sx, sy);
	glUniform2f(surfacePrograms->d_w_e, ex, ey);

	glBindBuffer(GL_ARRAY_BUFFER, surfaceBuffers->rectangle);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


// Actual Rendering

void render_surface(struct Surface* surf)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(surfacePrograms->backgroundProgram);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	
	SURF_ITERATE(struct Window, surf->wins, win)
	{
		drawWindowBuffer(win->sx, win->sy,
				win->ex, win->ey);
	}
//	drawWindowBuffer(100.0f, 100.0f, 300.0f, 200.0f);
	drawMenuBar();
	drawCursorBuffer(surf->cursor);

}
