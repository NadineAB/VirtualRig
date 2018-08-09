#ifndef MESH_GL_H
#define MESH_GL_H
#pragma warning (disable: 4002)
#pragma warning (disable: 4003)
// xml serialization related imports
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/base_object.hpp>
#include <GL/glew.h>
#include "mesh.h"
// Opengl related imports
#include <wrap/gl/trimesh.h>
#include <wrap/gl/decorator.h>
#include <wrap/gl/shaders.h>
/*! 
* \brief:
* class used to load and draw the mesh , init all texturing material 
* and shader related
* coded by Marco Fratarcangeli
*/
class CMeshGL : public CMesh
{
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		// serialize base class information
		ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(CMesh);
		ar & BOOST_SERIALIZATION_NVP(vs);
		ar & BOOST_SERIALIZATION_NVP(fs);
	}

	// param related to load and init shader
	ProgramVF * shp;  
	bool shaders_inited;
	bool textures_inited;
	void InitShader();
	void InitTextures();


public:
	// render mode
	enum DrawMode{SMOOTH = 0, POINTS, WIRE, FLATWIRE, HIDDEN, FLAT, SHADER};
	/// the current drawmode
	DrawMode drawmode;

	string vs;
	string fs;
	bool is_visible;

	CMeshGL(); // constrcut
	~CMeshGL (){} // desrctor
	string Init(); // which init texturing shader , import model of type xml file 
	void Draw(); // draw / render the 3d model with specific render mode
	// the active mesh opengl wrapper
	GlTrimesh<CMesh> glw;

	/// the active mesh decorator (bbox, normals, etc)
	MeshDecorator<CMesh> md;

};
#endif