#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H
// include after gl.h
#include <GL/glew.h>
#include <map>
#include <string>
using namespace std;
class TextureManager
{
	// this class manages textures in memory;
	// avoids to load multiple times the same texture and provide a function to release them

	// NOTE; it does not implement reference counting => release must be called iff ALL the 
	// objects sharing the textures have been deallocated

	// TODO: a texture manager with reference counts

	typedef map <string, GLuint>::iterator MapIterType;
	map<string, GLuint> textures;

	TextureManager();
	int Load(const string & filename) const;

	static TextureManager* m_inst;

public:
	static TextureManager* Inst();

	~TextureManager();

	// return false if loading fails
	bool Load(string filename, GLuint & id);
	void Release();
};

#endif // __TEXTURE_MANAGER__
