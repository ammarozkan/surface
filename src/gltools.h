
char* loadFile(char* filepath)
{
	FILE* file = fopen(filepath,"r");
	if(!file) {
		printf("File couldn't be opened. %s\n",strerror(errno));
		return NULL;
	}
	char c;
	unsigned int textsize = 1024;
	char* tresult = malloc(textsize);
	unsigned int counter = 0;
	printf("ReadingStart\n");
	while(fread(&c, 1, 1, file) == 1) {
		tresult[counter] = c;
		counter+=1;
		if(counter >= textsize) {
			char* newresult = malloc(textsize*2);
			memcpy(newresult,tresult,textsize);
			free(tresult);
			tresult = newresult;
			textsize*=2;
		}
	}
	tresult[counter] = '\0';

	char* result = malloc(counter+1);
	memcpy(result, tresult, counter+1);
	free(tresult);
	fclose(file);

	printf("LOADEDFILE:\n%s\nEND\n",result);
	return result;
}

GLuint create2dBuffer(float* buffer, unsigned int size)
{
	GLuint result;
	glGenBuffers(1, &result);
	glBindBuffer(GL_ARRAY_BUFFER, result);
	glBufferData(GL_ARRAY_BUFFER, size,
			buffer, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
			sizeof(float)*2, 0);
	return result;
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
getShaderFromFile(char* path, GLenum shaderType)
{
	char* src = loadFile(path);
	if(src == NULL) {
		printf("loadFile returned NULL.\n");
		return 0;
	}
	GLuint result = getShader(src, shaderType);
	free(src);
	return result;
}

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

GLuint
getProgram(GLuint vertexShader, GLuint fragmentShader)
{
	GLuint programid;

	programid = glCreateProgram();
	if(programid == 0) {
		printf("Program couldn't be created."
			" peace. GLERR:%i\n",glGetError());
		return 0;
	}

	glAttachShader(programid, vertexShader);
	glAttachShader(programid, fragmentShader);

	glLinkProgram(programid);


	return programid;

}

int
linkProgram(GLuint programid)
{	
	GLuint ivRes;

	glLinkProgram(programid);
	glGetProgramiv(programid, GL_LINK_STATUS, &ivRes);
	if(ivRes == GL_FALSE) {
		printf("Program couldn't be linked."
			"peace. GLERR:%i\n",glGetError());
		return 0;
	}
	return 1;
}
