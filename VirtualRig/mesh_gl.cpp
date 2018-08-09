#include "mesh_gl.h"
#include "texture_manager.h"

#include <vcg/space/color4.h>

CMeshGL::CMeshGL()
{
	//CMesh();
	Init();
	shp = 0;
	/*fs.assign(0);
	vs.assign(0);*/
	shaders_inited= false;
	textures_inited=false;
	is_visible = true;
	drawmode = FLATWIRE;//SHADER;
}
// initlize the shader and texturing related to the model in order to draw it 
string CMeshGL::Init()
{
	string err_msg = CMesh::Init();
	if (!err_msg.empty())
		return err_msg;

	// Initialize the opengl wrapper
	glw.m = this;
	glw.SetHint(GLW::HNUseVArray);
	glw.Update();
	// Decorator like noraml , tangent bounding box here just the box corner used and axis nothing else
	md.Init(glw.m);
	md.SetDecorator(/*DT_SHOW_NORMALS | DT_SHOW_TANGENTS | */DT_SHOW_BOX_CORNERS | DT_SHOW_AXIS /*|  DT_SHOW_QUOTED_BOX*/);

	// init textures
	textures.clear();
	for (map<string, string>::iterator it = material.textures.begin();
		it != material.textures.end(); it++)
		textures.push_back((*it).second);

	// this shall be inited when a opengl context is surely created
	shaders_inited = false;
	textures_inited = false;

	return string();
}
/* Render Mesh :
* param: load gl drawing attributes
*Camera matrix model view and other veriable */
void CMeshGL::Draw()
{
	if (!is_visible)
		return;

	else
	{
		glEnable(GL_BLEND);
		//		glDisable(GL_DEPTH_TEST);
	}
	glDisable(GL_LIGHTING);

	if (drawmode == SHADER)
	{
		if (!textures_inited)
			InitTextures();
		if (!shaders_inited)
			InitShader();

		// compute point of view
		GLfloat mm[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, mm);

		Matrix44<CMesh::ScalarType> mv(mm);
		Transpose(mv);
		//Invert(mv);

		Point4<CMesh::ScalarType> pov = mv * Point4<CMesh::ScalarType>(0, 0, 0, 1);
		Point4<CMesh::ScalarType> cdir = mv * Point4<CMesh::ScalarType>(0, 0, 1, 1);
		cdir = (cdir - pov).Normalize();

		Point3<CMesh::ScalarType> pov3(pov[0], pov[1], pov[2]);
		Point3<CMesh::ScalarType> cdir3(cdir[0], cdir[1], cdir[2]);

#if 1
		//printf("%.3f\t%.3f\t%.3f\t\t", pov3[0], pov3[1], pov3[2]); 
		//printf("%.3f\t%.3f\t%.3f\n", cdir3[0], cdir3[1], cdir3[2]); 
		shp->Bind();
		shp->prog.Uniform("CamPos", pov3);
		shp->prog.Uniform("CamDir", cdir3);
		shp->Unbind();


		// Tangent generated previously, no need to pass the binormal, a cross product in the vs will generate it
		const GLint loc = glGetAttribLocation(shp->prog.ObjectID(), "tangent");
		glEnableVertexAttribArray(loc);
		glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, sizeof(CMesh::VertexType), &(glw.m->vert.begin()->Tangent()));
		shp->Bind();
		glw.Draw<vcg::GLW::DMSmooth, vcg::GLW::CMNone, GLW::TMPerWedgeMulti>();
		shp->Unbind();
		glDisableVertexAttribArray(loc);
#endif
	}

	if (drawmode == FLATWIRE)
	{

		//	glColor4f(1, 1, 1, 1);
		glColor(color);
		//glEnable(GL_LIGHTING);
		// drawing using glw in vcg
		glw.Draw<vcg::GLW::DMFlatWire,   vcg::GLW::CMNone/*vcg::GLW::CMPerVert*/, vcg::GLW::TMNone> ();
		md.Decorate();

		// draw vertices without weights
		glPointSize(5.f);
		glBegin(GL_POINTS);
		glColor4f(1, 1, 1, 1);

		if (!morphs.empty())	
			for (int i = 0; i < vert.size(); i++)
				if (mapping[i] == -1)
					glVertex(vert[i].P());

		//for (VertexIterator vi = vert.begin(); vi != vert.end(); vi++)
		//{
		//	if (vi->bone_weight.empty())
		//		glVertex(vi->P());
		//}
		glEnd();
	}

	//DrawGrid();

}
/* init textureing  
*/
void CMeshGL::InitTextures()
{
	if(!textures.empty() && glw.TMId.empty())
	{
		for(unsigned int i = 0; i < textures.size(); ++i)
		{
			GLuint id = -1;
			bool res = TextureManager::Inst()->Load(textures[i].c_str(), id);
			if (!res) 
				printf("Failure of loading texture %s\n", textures[i].c_str());
			glw.TMId.push_back(id);
		}
	}
	textures_inited = true;
}
/* init shader
*/
void CMeshGL::InitShader()
{
	assert(!vs.empty());
	assert(!fs.empty());

	// Read and compile shader program(s)
	if (shp)
		delete shp;

	shp = new ProgramVF();
	shp->LoadSources(vs.c_str(), fs.c_str());
	bool res = shp->prog.Link();
	printf("%s\n", shp->prog.InfoLog().c_str());

	float shininess = 50.0f;
	shp->Bind();
	shp->prog.Uniform("shininess", shininess);
	shp->prog.Uniform("color", color);

	// load the texture into the shader (if the shader supports them)
	// each texture is loaded in the proper sampler (diffuse, specular, etc)
	int index = 0;
	for (map<string, string>::iterator it = material.textures.begin();
		it != material.textures.end(); it++)
	{
		shp->prog.Uniform(it->first.c_str(), (GLint)glw.TMId[index++]);	// upload texture maps
		string text_enabled = it->first + "_enabled";
		shp->prog.Uniform(text_enabled.c_str(), (GLfloat)1.0);		// tell the shader which textures are enabled
	}

	shp->Unbind();

	shaders_inited = true;
	cout<< " Shader calling ";
}

