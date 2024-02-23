
// We're trying this define later.
// or removing it permanently
//#define GL_GLEXT_PROTOTYPES 1
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

static struct
{
	GLuint windowBase;
}*surfaceBuffers;

static struct
{
	GLuint dShader_vertex;
	GLuint dShader_fragment;
	GLuint dProgram;
}*surfacePrograms;

const char* dVertex_src =
	"attribute vec2 position; "
	"vec2 screen = vec2(800,600);"
	"vec2 w_pos = vec2(100,100);"
	"vec2 w_size = vec2(100,300);"
	"vec2 result;"
	"void main()"
	"{"
	"vec2 normd = vec2(position.x*w_size.x + w_pos.x, position.y*w_size.y + w_pos.y);"
	"normd = vec2(normd.x/screen.x, normd.y/screen.y);"
	"result = vec2(2.0*normd.x-1.0, "
		"1.0-2.0*normd.y);"
	"gl_Position = vec4(result, 0.0, 1.0);"
	"}";

const char* dFragment_src =
	"void main()"
	"{"
	"gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);"
	"}";

void
compileShader(GLuint shaderid, const char* source)
{
	glShaderSource(shaderid, 1, 
			&source, NULL);

	char infoLog[256]; 
	GLsizei infoLength = 255;
	
	glGetShaderInfoLog(shaderid, sizeof(infoLog),
			&infoLength, infoLog);
	printf("CompileLog:%s\n",infoLog);
}

GLuint getShader(const char* src, GLenum type);
GLuint getProgram(GLuint vert,GLuint frag);

void
initBuffers()
{
	GLuint dShader_vertex;
	GLuint dShader_fragment;
	
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

}

void
destroyBuffers()
{
	glDeleteBuffers(1,&surfaceBuffers->windowBase);
	free(surfaceBuffers);
}

int
initPrograms()
{
	GLint ivRes;

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

	surfacePrograms->dShader_vertex = 
		getShader(dVertex_src, GL_VERTEX_SHADER);
	
	surfacePrograms->dShader_fragment = 
		getShader(dFragment_src, GL_FRAGMENT_SHADER);
	
	if(!(surfacePrograms->dShader_vertex && 
				surfacePrograms->dShader_fragment))
	{
		printf("One of the shaders are not "
			"did their job correctly.\n");
		return 0;
	}
	else {
		printf("I think shaders will work.\n");
	}
	surfacePrograms->dProgram = 
		getProgram(surfacePrograms->dShader_vertex,
				surfacePrograms->dShader_fragment);

	glReleaseShaderCompiler();

	return 1;
}

void
destroyPrograms()
{
	glDeleteProgram(surfacePrograms->dProgram);

	glDeleteShader(surfacePrograms->dShader_vertex);
	glDeleteShader(surfacePrograms->dShader_fragment);
}

GLuint
getShader(const char* src,GLenum shaderType)
{
	char infoLog[256]; 
	GLsizei infoLength = 255;
	GLuint sid;
	
	if((sid = glCreateShader(shaderType)) == 0) {
		printf("Shader creation problem."
			"peace. GLERR:%i\n",glGetError());
		return 0;
	}
	glShaderSource(sid, 1, &src, NULL);	
	glCompileShader(sid);

	
	glGetShaderInfoLog(sid, sizeof(infoLog),
			&infoLength, infoLog);
	printf("CompileLog:%s\n",infoLog);

	GLint compileStat;
	glGetShaderiv(sid, GL_COMPILE_STATUS, &compileStat);
	
	if(compileStat != GL_TRUE) {
		printf("Compile NOT succesful brah."
			"peace. GLERR:%i\n",glGetError());
		return 0;
	} else {	
		printf("Compile succesful brah."
			"peace. GLERR:%i\n",glGetError());
	}

	return sid;
}

GLuint
getProgram(GLuint vertexShader, GLuint fragmentShader)
{
	GLuint programid, ivRes;

	programid = glCreateProgram();
	if(programid == 0) {
		printf("Program couldn't be created."
			" peace. GLERR:%i\n",glGetError());
		return 0;
	}

	glAttachShader(programid, vertexShader);
	glAttachShader(programid, fragmentShader);

	glLinkProgram(programid);

	glGetProgramiv(programid, GL_LINK_STATUS, &ivRes);
	if(ivRes == GL_FALSE) {
		printf("Program couldn't be linked."
			"peace. GLERR:%i\n",glGetError());
		return 0;
	}

	return programid;

}

void
drawWindowBuffer()
{
	glUseProgram(surfacePrograms->dProgram);
	glBindBuffer(GL_ARRAY_BUFFER, surfaceBuffers->windowBase);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


// Actual Rendering

void render_surface(struct Surface* surface)
{
	glClearColor(0.2, 0.2, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	drawWindowBuffer();
}
