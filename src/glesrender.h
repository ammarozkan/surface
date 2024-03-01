
// We're trying this define later.
// or removing it permanently
//#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "gltools.h"

#include "glestext.h"

static struct
{
	GLuint rectangle, rectangleVAO;
	GLuint vCursor, vCursorVAO;
}*surfaceBuffers;

static struct
{
	GLuint windowVertex;
	GLuint windowFragment;
	GLint d_w_s;
	GLint d_w_e;

	GLuint cursorVertex;
	GLuint cursorFragment;
	GLint CursorProgram_cursorPos;
	GLint CursorProgram_screen_size;

	GLuint backgroundVertex;
	GLuint backgroundFragment;

	GLuint menubarVertex;
	GLuint menubarFragment;


	GLuint windowProgram;
	GLuint cursorProgram;
	GLuint backgroundProgram;
	GLuint menubarProgram;
}*surfacePrograms;

FT_Library freetype2;
FT_Face systemFace;


int
initFonts()
{
	if(!initFreetype()) 
		return 0;
	else if(!initSystemFace("fonts/thebrooklynsmooth-bold-demo.ttf")) 
		return 0;
	return 1;
}

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
	glGenVertexArrays(1, &surfaceBuffers->rectangleVAO);
	glBindVertexArray(surfaceBuffers->rectangleVAO);
	surfaceBuffers->rectangle = create2dBuffer(rectangle, 
			sizeof(rectangle));

	glGenVertexArrays(1, &surfaceBuffers->vCursorVAO);
	glBindVertexArray(surfaceBuffers->vCursorVAO);
	surfaceBuffers->vCursor = create2dBuffer(vCursor, sizeof(vCursor));

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
	if(surfacePrograms->CursorProgram_cursorPos == -1) {
		printf("cursorPos uniform not found in cursor program.\n");
		return 0;
	}

	surfacePrograms->CursorProgram_screen_size = glGetUniformLocation(ids[2], "screen_size");
	if(surfacePrograms->CursorProgram_cursorPos == -1) {
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
	glBindAttribLocation(ids[2], 0, "model_pos");
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
	} else if(!initTextRenderer()) {
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


int
initProgramScreenSize(uint32_t width, uint32_t height)
{
	if(!setProgramScreenSize(surfacePrograms->windowProgram,
			width, height)) printf("Cannot window ws\n");	

	else if(!setProgramScreenSize(surfacePrograms->menubarProgram,
			width, height)) printf("Cannot menubar ws\n");	

	else if(!setProgramScreenSize(surfacePrograms->cursorProgram,
			width, height)) printf("Cannot cursor ws\n");	

	else if(!textRendererScreenSize(width, height)) {
		printf("Text Renderer not found screen_size uniform.\n");
	} 
	
	else return 1;

	printf("Program screen size init not succesful!\n");
	return 0;
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
	glBindVertexArray(surfaceBuffers->vCursorVAO);
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
	glBindVertexArray(surfaceBuffers->rectangleVAO);
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

	renderASCIIText("Hello World", systemChars, 100.0f, 100.0f, 1.0f);

}
