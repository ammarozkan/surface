#include <ft2build.h>
#include FT_FREETYPE_H

struct FTCharacter {
	unsigned int TextureID;
	GLint width, rows;
	GLint bearing_x, bearing_y;
	unsigned int advance;
};

// Maybe struct TextRenderer here?

FT_Library freetype2;
struct FTCharacter* systemChars;

struct FTCharacter* // 128 characters
loadSystemFont128ASCII(FT_Face face)
{
	struct FTCharacter* result = malloc(sizeof(struct FTCharacter) * 128);
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
		result[c].TextureID = texture;
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
