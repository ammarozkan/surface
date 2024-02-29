#include <ft2build.h>
#include FT_FREETYPE_H

struct FTCharacterBuffer {
	unsigned int textureID;
	GLint width, rows;
	GLint bearing_x, bearing_y;
	unsigned int advance;
} *systemChars;

static struct {
	GLuint vertex, fragment;
	GLuint program;
	GLint color;
	
	GLint VAO, VBO;
}TextRenderer;

// Maybe struct TextRenderer here?

FT_Library freetype2;

struct FTCharacterBuffer* // 128 characters
loadSystemFont128ASCII(FT_Face face)
{
	struct FTCharacterBuffer* result = malloc(sizeof(struct FTCharacterBuffer) * 128);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for(unsigned char c = 0; c < 128; c++) {
		if(FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			printf("Glyph %u cannot be loaded from system font.\n",
					c);
			continue;
		}

		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
				GL_TEXTURE_2D,
				0,
				GL_LUMINANCE,
				face->glyph->bitmap.width,
				face->glyph->bitmap.rows,
				0,
				GL_LUMINANCE,
				GL_UNSIGNED_BYTE,
				face->glyph->bitmap.buffer
		);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, 
				GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
				GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
				GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, 
				GL_LINEAR);
		
		FT_GlyphSlot glyph = face->glyph;
		result[c].textureID = texture;
		result[c].width = glyph->bitmap.width;
		result[c].rows = glyph->bitmap.rows;
		result[c].bearing_x = glyph->bitmap_left;
		result[c].bearing_y = glyph->bitmap_top;
		result[c].advance = glyph->advance.x;
		
	}
	return result;
}

int
initFreetype()
{
	if(FT_Init_FreeType(&freetype2))
	{
		printf("FreeType cannot init.\n");
		return 0;
	}
	return 1;
}

int
destroyFreetype()
{
	int err = FT_Done_FreeType(freetype2);
	if(err!=0) {
		printf("While destroying ft2, fterr:%u\n",err);
		return 0;
	}
	return 1;
}

int
initSystemFace(char* path)
{
	FT_Face systemFace;

	if(!initFreetype()) return 0;

	if(FT_New_Face(freetype2, path,
				0, &systemFace))
	{
		printf("Font cannot be retrieved.\n");
		return 0;
	}
	FT_Set_Pixel_Sizes(systemFace, 0, 48);
	if(FT_Load_Char(systemFace, 'X', FT_LOAD_RENDER))
	{
		printf("Failed to load example 'X' Glyph.\n");
		return 0;
	}

	systemChars = loadSystemFont128ASCII(systemFace);

	FT_Done_Face(systemFace);
	return 1;
}

int
initTextRenderer()
{
	glGenVertexArrays(1, &TextRenderer.VAO);
	glBindVertexArray(TextRenderer.VAO);
	glGenBuffers(1, &TextRenderer.VBO);
	glBindBuffer(GL_ARRAY_BUFFER, TextRenderer.VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, 
			NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, 
			GL_FALSE, 4*sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, 
			GL_FALSE, 4*sizeof(float), 2*sizeof(float));

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	int* ids = initClassicalProgram(
			"shaders/textVertex.glsl",
			"shaders/textFragment.glsl");
	if(ids == NULL) {
		printf("text program cannot be inited.\n");
		return 0;
	}
	glBindAttribLocation(ids[2], 0, "model_pos");
	glBindAttribLocation(ids[2], 1, "texture_coord_attrb");
	if(!linkProgram(ids[2])) goto erralloc;
	TextRenderer.vertex = ids[0];
	TextRenderer.fragment = ids[1];
	TextRenderer.program = ids[2];

	TextRenderer.color = glGetUniformLocation(TextRenderer.program,
			"textColor");
	if(TextRenderer.color == -1) {
		int glerr = glGetError();
		printf("color uniform not found in text program:%u:%u\n",glerr,GL_INVALID_VALUE);
		if(glerr == GL_INVALID_VALUE) printf("Program id input:%u\n",TextRenderer.program);
		return 0;
	}
	
	return 1;
erralloc:
	free(ids);
	return 0;
}

void
renderASCIIText(char* text, struct FTCharacterBuffer* preloads, float x, float y, float scale)
{
	glUseProgram(TextRenderer.program);
	glUniform4f(TextRenderer.color, 1.0, 1.0, 1.0, 1.0);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(TextRenderer.VAO);

	for(unsigned int i = 0; text[i] != '\0' ; i += 1) {
		char c = text[i];
		struct FTCharacterBuffer chbuf = preloads[c];

		float xpos = x + chbuf.bearing_x * scale;
		float ypos = y - (chbuf.rows - chbuf.bearing_y) * scale;

		float w = chbuf.width * scale;
		float h = chbuf.rows * scale;

		// Vertex Buffer for each character
		float vertices[4][4]  = {
			{xpos, 		ypos,		0.0f, 	1.0f},
			{xpos, 		ypos + h,	0.0f, 	0.0f},
			{xpos + w,	ypos,		1.0f,	1.0f},
			{xpos + w,	ypos + h,	1.0f,	0.0f}
		};
		glBindTexture(GL_TEXTURE_2D, chbuf.textureID);
		glBindBuffer(GL_ARRAY_BUFFER, TextRenderer.VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		x += (chbuf.advance >> 6) * scale; // bitshift by 6 to get value in pixels
	}
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}
