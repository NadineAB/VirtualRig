#include "texture_manager.h"
#include "FreeImage.h"

#include <assert.h>

TextureManager* TextureManager::m_inst(0);

TextureManager* TextureManager::Inst()
{
	if(!m_inst)
		m_inst = new TextureManager();

	return m_inst;
}

TextureManager::TextureManager()
{
	// call this ONLY when linking with FreeImage as a static library
#ifdef FREEIMAGE_LIB
	FreeImage_Initialise();
#endif
}

TextureManager::~TextureManager()
{
	// call this ONLY when linking with FreeImage as a static library
#ifdef FREEIMAGE_LIB
	FreeImage_DeInitialise();
#endif

	Release();
	m_inst = 0;
}

// return false if loading fails
bool TextureManager::Load(string filename, GLuint & id)
{
	MapIterType iter = textures.find(filename);
	if (iter == textures.end())
	{
		// the texture is not stored so load it from disk
		id = Load(filename);
		if (id == -1)	// something failed ... a file is missing?
			assert(false);
		else
			textures[filename] = id;
	}
	else
	{
		// texture is already in memory
		id = iter->second;
	}

	if (id == -1)
		return false;

	return true;
}

void TextureManager::Release()
{
	// here all the textures are deallocated
	// it must be called AFTER all the objects using the textures have been destroyed

	for (MapIterType it = textures.begin(); it != textures.end(); it++)
	{
		glDeleteTextures(1, &(it->second));
		printf("released texture %s from memory", it->first.c_str());
	}

	textures.clear();
}


int TextureManager::Load(const string & filename) const
{
	GLuint res = -1;

	//image format
	FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
	//pointer to the image, once loaded
	FIBITMAP *dib(0);
	//pointer to the image data
	BYTE* bits(0);
	//image width and height
	unsigned int width(0), height(0);
	//OpenGL's image ID to map to
	GLuint gl_texID;

	//check the file signature and deduce its format
	fif = FreeImage_GetFileType(filename.c_str(), 0);
	//if still unknown, try to guess the file format from the file extension
	if(fif == FIF_UNKNOWN) 
		fif = FreeImage_GetFIFFromFilename(filename.c_str());
	//if still unkown, return failure
	if(fif == FIF_UNKNOWN)
		return false;

	//check that the plugin has reading capabilities and load the file
	if(FreeImage_FIFSupportsReading(fif))
		dib = FreeImage_Load(fif, filename.c_str());
	//if the image failed to load, return failure
	if(!dib)
		return false;

	//retrieve the image data
	bits = FreeImage_GetBits(dib);
	int bpp = FreeImage_GetBPP(dib);
	//get the image width and height
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);
	//if this somehow one of these failed (they shouldn't), return failure
	if((bits == 0) || (width == 0) || (height == 0))
		return false;

	GLenum format = -1;
	if (bpp == 24)
		format = GL_BGR;
	if (bpp == 32)
		format = GL_BGRA;
	assert(format != -1);

	glEnable(GL_TEXTURE_2D);
	glGenTextures( 1, &res );
	glBindTexture( GL_TEXTURE_2D, res );
	glTexImage2D( GL_TEXTURE_2D, 0, 3, width, height, 0, format, GL_UNSIGNED_BYTE, bits );
	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, width, height, format, GL_UNSIGNED_BYTE, bits );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glDisable(GL_TEXTURE_2D);

	printf("loaded texture %s\tw %i\th %i\n", filename.c_str(), width, height);

	return res;
}
