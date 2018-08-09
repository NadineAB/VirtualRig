#pragma warning (disable: 4002)
#pragma warning (disable: 4003)
#include "mesh.h"
#include "Edge.h"
#include "timer.h"
#include "triangle.h"
#include <assert.h>
//#define USE_PROFILE		// undef to disable profiling
#include "profile.h"
using namespace std;
/*****************************************************************/
/////////////////// lighting /////////////////////////////////////
GLfloat AWHITE[] = {0.3, 0.3, 0.3};
GLfloat SWHITE[] = {0.3,0.3, 0.3};
GLfloat BROWN[] = {0.45f, 0.315f, 0.19f};
GLfloat DWHITE[] = {0.2f,0.2f, 0.2f};
GLfloat Blue[] = {0.0f,0.0f, 1.0f};
GLfloat Green[] = {0.0f,1.0f, 0.0f};
GLfloat Red[] = {1.0f,0.0f, 0.0f};
GLfloat RanRed[] = {0.8f,0.3f, 0.0f};
GLfloat RanGreen[] = {0.3f,0.7f, 0.0f};
// include headers that implement a archive in xml format
using namespace boost::filesystem;
/*! 
* \brief:
* method to assgin for each vetex  
* which is average the boneweight
* 1.bone sum weight
* 2.average them up
*/
void CVertex::NormalizeWeights()
{
	//	cout << Index() << ": " << weight.size() << endl;

	if (bone_weight.empty())
		return;

	float sum = 0;
	for (vector<CBoneWeight>::iterator it = bone_weight.begin(); it != bone_weight.end(); it++)
		sum += it->weight;

	for (vector<CBoneWeight>::iterator it = bone_weight.begin(); it != bone_weight.end(); it++)
		it->weight = it->weight / sum;

	// check for weights with sum magnitude very low (e.g. they are zero)
	if (sum < 0.1)
		bone_weight.clear();

}
/*! 
* \brief:
* Param : reviced object of boneweight
* which assgin each vertices with specific weight
*/
void CVertex::PushBoneWeight(const CBoneWeight & bw)
{
	for (vector<CBoneWeight>::iterator it = bone_weight.begin(); it != bone_weight.end(); it++)
	{
		if (it->bone == bw.bone)
			return;
	}

	bone_weight.push_back(bw);
}
/*!
/*  \brief: Construct initalize defult values
*/
CMesh::CMesh()
{
	pitch = opitch = 0;
	yaw = oyaw = 0;
	is_static = false;
	is_transparent = false;
	needs_grid = false;
	//cout << " here the mesh constrcut " ;
}
// delete any morphing/bending done on the mesh
CMesh::~CMesh()
{  
	for (int i = 0; i < morphs.size(); i++)
		delete morphs[i];
	//cout << " here the mesh constrcut " ;
}
/*! 
* \brief:
* load mesh/model from storage update noramls for face and for vertics
* adj faces and bounding box
* tangent
*/
/**************************************************************************** 
///////////////////////////// Performance Profiling \\\\\\\\\\\\\\\\\\\\\\\\\  
1. FIRST LOAD 3D MESH
a. READ XML FILE 
b. CMesh::Init() 
/*****************************************************************************/
string CMesh::Init()
{
	PROFILE_SAMPLE("Init_MESH");
	// load from file; assume filename storing the file to load
	float t0 = GetRealTime();
	int lm;
	//int err = vcg::tri::io::ImporterCTM<CMesh>::Open(*this, filename.c_str(), lm);
	/********** a. READ XML FILE ( load vertices, faces and normal ) ********/
	int err = vcg::tri::io::ImporterOBJ<CMesh>::Open(*this, filename.c_str(), lm);
	//if (err != 0)
	//	return vcg::tri::io::ImporterOBJ<CMesh>::ErrorMsg(err);
	//printf("loaded mesh %s\tv: %d f: %d tex: %d time: %.3f s\n", filename.c_str(), vert.size(), face.size(),face.size() * 3 * 2, GetRealTime() - t0);

	// adjacency
	vcg::tri::UpdateTopology<CMesh>::VertexFace(*this); //update adjacency face , first vertices then faces  
	vcg::tri::UpdateTopology<CMesh>::VertexFace(*this); //update adjacency Edge , first vertices then faces  
	vcg::tri::UpdateBounding<CMesh>::Box(*this); // bounding box
	vcg::tri::UpdateNormal<CMesh>::PerVertexNormalizedPerFace(*this); // vertice normal and face noraml

	///////////// assign index to vertices/////////////////////////
	for ( int i = 0; i < skin._particles.size(); i++)
		vert[i].Index() = i;

	return string();
}
// update the physical system
///////////////////////////// Performance Profiling \\\\\\\\\\\\\\\\\\\\\\\\\  
//3: MODEL:: update the physical system
//3.1 Time step
//a.AccumulateForces
//b.Verlet			// numerical integration with Verlet (the paper does it better)
//c.SatisfyConstraints
/*****************************************************************************/
void CMesh ::UpdateSkin() 
{
	PROFILE_SAMPLE("MESH_updateSkin");
	//printf( " in update skin");
	//skin.CalculateFaceNormals();
	skin.TimeStep();

}

// open the directory, load all the facial morph targets and store them in morphs[]
// finds the mapping between the mts and the mesh (body)
void CMesh::LoadMorphs(const string & dir, const string & key)
{
	assert(!dir.empty());
	assert(!key.empty());
	path p (dir.c_str());   

	if (exists(p))    // does p actually exist?
	{
		if (is_directory(p))      // is p a directory?
		{
			cout << "loading morph targets in " << p <<"\n";

			directory_iterator di = directory_iterator(p);
			for (; di != directory_iterator(); ++di)
			{
				string filename = di->path().string();
				if (filename.rfind(key) != string::npos)
				{
					printf("%s\r", filename.c_str());
					int lm;
					CMesh * mesh = new CMesh();
					int err = vcg::tri::io::ImporterOBJ<CMesh>::Open(*mesh, filename.c_str(), lm);
					morphs.push_back(mesh);
				}
			}
			cout << endl;
		}

		else
			cout << p << " exists, but is not a directory\n";
	}
	else
		cout << p << " does not exist\n";

	if (!morphs.empty())
		vcg::tri::UpdateBounding<CMesh>::Box(*morphs[0]);

	FindMorphMapping();
}

void CMesh::FindMorphMapping()
{
	assert(!morphs.empty());
	printf("finding mapping with morph targets\n");

	CMesh * mt0 = morphs[0];

	for (int i = 0; i < vert.size(); i++)
	{
		//		CoordType & p_i = vert[i].P();
		Point3d p_i;
		p_i.Import<float>(vert[i].P());
		printf("vert %d\r", i);

		double min_dist = 10000;
		int res = -1;
		for (int j = 0; j < mt0->vert.size(); j++)
		{
			//			CoordType & p_j = mt0->vert[j].P();
			Point3d p_j;
			p_j.Import<float>(mt0->vert[j].P());
			double dist = (p_i - p_j).SquaredNorm();

			if (dist < min_dist)
			{
				res = j;
				min_dist = dist;
			}
		}

		if (min_dist > bbox.Diag() / 1000.f)
			res = -1;

		mapping.push_back(res);
	}
	cout << endl;

	assert(mapping.size() == vert.size());
}

void CMesh::ApplyMorph(int index)
{
	assert (index < morphs.size());

	CMesh & mt = *morphs[index];

	for (int i = 0; i < mapping.size(); i++)
	{
		int j = mapping[i];

		if (j == -1)
			continue;

		assert(j < mapping.size());
		vert[i].P() = mt.vert[j].P();
	}
}
/* I should change that I draw the character */
void CMesh :: DrawSkin()
{
	glPushMatrix();
	//glPolygonMode(GL_FRONT ,GL_FILL);
	glLightfv(GL_LIGHT0, GL_AMBIENT, AWHITE);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, DWHITE);
	glLightfv(GL_LIGHT0, GL_SPECULAR,SWHITE);
	glMaterialfv(GL_FRONT, GL_AMBIENT, AWHITE);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,DWHITE);
	glMaterialfv(GL_FRONT, GL_SPECULAR,SWHITE);
	glMaterialf(GL_FRONT, GL_SHININESS, 4);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat lightPosition[] = {5,5,5,1};
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	//glShadeModel(GL_FLAT);
	glPushMatrix();
	glPolygonMode(GL_FRONT_AND_BACK ,GL_FILL);
	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glEnableClientState (GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_DOUBLE, sizeof(Particle), &(skin.GetParticleAt(0)->pos.x));
	glNormalPointer(GL_FLOAT, 0, skin.VerNorms);
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	/* draw geometry (vertex arrays) */
	// draw filled triangles
	//glColor3f(0.497059f,0.394706f,0.174706f);
	glColor3f(0.48f, 0.315f, 0.19f);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,BROWN);
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	glDrawElements(GL_TRIANGLES , skin.nb_tri*3, GL_UNSIGNED_INT, &skin._faces[0]);
	/* draw geometry (vertex arrays) */
	glPopMatrix();
	glDisableClientState (GL_NORMAL_ARRAY);
	glDisableClientState (GL_VERTEX_ARRAY);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glPolygonMode(GL_FRONT_AND_BACK ,GL_FILL);
	/* draw wireframe */
	glPushMatrix();
	
	//glPolygonMode(GL_FRONT_AND_BACK ,GL_FILL);
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glEnableClientState (GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_DOUBLE, sizeof(Particle), &(skin.GetParticleAt(0)->pos.x));
	glNormalPointer(GL_FLOAT, 0, skin.VerNorms);
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	/* draw geometry (vertex arrays) */
	// draw filled triangles
	//glColor3f(0.497059f,0.394706f,0.174706f);
	glLineWidth(2.8);
	glColor3f(0.48f, 0.415f, 0.29f);
	glDrawElements(GL_TRIANGLES , skin.nb_tri*3, GL_UNSIGNED_INT, &skin._faces[0]);
	glPopMatrix();
	// glDisableClientState (GL_NORMAL_ARRAY);
	glDisableClientState (GL_VERTEX_ARRAY);
	glDisableClientState (GL_NORMAL_ARRAY);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	glPolygonMode(GL_FRONT_AND_BACK ,GL_FILL);
	glPopMatrix();

}//
void CMesh :: DrawWireSkin()
{    
	glPushMatrix();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glClearColor(0.95, 0.95, 0.95,1.0);
	glPolygonMode(GL_FRONT ,GL_FILL);
	glLightfv(GL_LIGHT0, GL_AMBIENT, AWHITE);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, DWHITE);
	glLightfv(GL_LIGHT0, GL_SPECULAR,SWHITE);
	glMaterialfv(GL_FRONT, GL_AMBIENT, AWHITE);
	glMaterialfv(GL_FRONT, GL_DIFFUSE,DWHITE);
	glMaterialfv(GL_FRONT, GL_SPECULAR,SWHITE);
	glMaterialf(GL_FRONT, GL_SHININESS, 15);
	glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
	GLfloat lightPosition[] = {5,5,5,1};
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	//glShadeModel(GL_FLAT);
	
	//glPolygonMode(GL_FRONT_AND_BACK ,GL_FILL);
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glEnableClientState (GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_DOUBLE, sizeof(Particle), &(skin.GetParticleAt(0)->pos.x));
	glNormalPointer(GL_FLOAT, 0, skin.VerNorms);
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	/* draw geometry (vertex arrays) */
	// draw filled triangles
	//glColor3f(0.497059f,0.394706f,0.174706f);
	glColor3f(0.45f, 0.315f, 0.19f);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,BROWN);
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	glDrawElements(GL_TRIANGLES , skin.nb_tri*3, GL_UNSIGNED_INT, &skin._faces[0]);
	
	// glDisableClientState (GL_NORMAL_ARRAY);
	glDisableClientState (GL_VERTEX_ARRAY);
	glDisableClientState (GL_NORMAL_ARRAY);
	glDisable(GL_LIGHTING);
	//glDisable(GL_LIGHT0);
	//glPolygonMode(GL_FRONT_AND_BACK ,GL_LINE);
	glPopMatrix();

}//
void CMesh :: DrawSkinError()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_CLEAR);
	glClearColor(0.95, 0.95, 0.95,1.0);
	glPushMatrix();
	glEnable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);
	//glShadeModel(GL_FLAT);
	if (skin._particles.empty())
		return;
	//glDrawBuffer(GL_FRONT_AND_BACK);
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glBegin (GL_TRIANGLES);
	for (int i = 0; i <skin.nb_tri; i++)
	{ 

		int nV = i*3;
		int i1 = skin._faces[nV];
		int i2 = skin._faces[nV+1];
		int i3 = skin._faces[nV+2];

		if(skin._particles[i1].errorpos==0.0|| skin._particles[i1].errorpos<=0.000000001)
		{
			
			glColor3f(0.0,0.0,1.0); // blue
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,Blue);
			glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
			Vector3 a = skin._particles[i1].pos;
			Vec3f na = skin.VerNorms[i1];
			glNormal3f(na.x(),na.y(),na.z());
			glVertex3f(a.x,a.y,a.z);
			
		}
		else if(skin._particles[i1].errorpos>0.0000001)
		{
			float rndm = 1-(float) (rand() / RAND_MAX);
			glColor3f(0.8,0.3,0.0); // red
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,RanRed);
			glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
			Vector3 b = skin._particles[i1].pos;
			Vec3f nb = skin.VerNorms[i1];
			glNormal3f(nb.x(),nb.y(),nb.z());
			glVertex3f(b.x,b.y,b.z);
			
		}
		else if(skin._particles[i1].errorpos>=0.000000001 && skin._particles[i1].errorpos<=0.0000001)
		{
			float rndm = 1-(float) (rand() / RAND_MAX);
			glColor3f(rndm,0.9,0.0); // green
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,RanGreen);
			glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
			Vector3 c = skin._particles[i1].pos;
			Vec3f nc = skin.VerNorms[i1];
			glNormal3f(nc.x(),nc.y(),nc.z());
			glVertex3f(c.x,c.y,c.z);
			
		}
		if(skin._particles[i2].errorpos==0.0|| skin._particles[i2].errorpos<=0.000000001)
		{
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,Blue);
			glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
			glColor3f(0.0,0.0,1.0); // blue
			Vector3 a = skin._particles[i2].pos;
			Vec3f na = skin.VerNorms[i2];
			glNormal3f(na.x(),na.y(),na.z());
			glVertex3f(a.x,a.y,a.z);
			
		}
		else if(skin._particles[i2].errorpos>0.0000001)
		{
			float rndm = 1-(float) (rand() / RAND_MAX);
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,RanRed);
			glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
			glColor3f(0.8,0.3,0.0); // red
			Vector3 b = skin._particles[i2].pos;
			Vec3f nb = skin.VerNorms[i2];
			glNormal3f(nb.x(),nb.y(),nb.z());
			glVertex3f(b.x,b.y,b.z);
			
		}
		else if(skin._particles[i2].errorpos>=0.000000001 && skin._particles[i2].errorpos<=0.0000001)
		{
			float rndm = 1-(float) (rand() / RAND_MAX );
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,RanGreen);
			glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
			glColor3f(rndm,0.8,0.0); // green
			Vector3 c = skin._particles[i2].pos;
			Vec3f nc = skin.VerNorms[i2];
			glNormal3f(nc.x(),nc.y(),nc.z());
			glVertex3f(c.x,c.y,c.z);
			
		}
		if(skin._particles[i3].errorpos==0.0|| skin._particles[i3].errorpos<=0.000000001)
		{
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,Blue);
			glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
			glColor3f(0.0,0.0,1.0); // blue
			Vector3 a = skin._particles[i3].pos;
			Vec3f na = skin.VerNorms[i3];
			glNormal3f(na.x(),na.y(),na.z());
			glVertex3f(a.x,a.y,a.z);
			
		}
		else if(skin._particles[i3].errorpos>0.0000001)
		{
			float rndm = 1-(float) (rand() / RAND_MAX );
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,RanRed);
			glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
			glColor3f(0.8,rndm,0.0); // red
			Vector3 b = skin._particles[i3].pos;
			Vec3f na = skin.VerNorms[i3];
			glNormal3f(na.x(),na.y(),na.z());
			glVertex3f(b.x,b.y,b.z);
			
		}
		else if(skin._particles[i3].errorpos>=0.000000001 && skin._particles[i3].errorpos<=0.0000001)
		{
			float rndm = 1-(float) (rand() / RAND_MAX);
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,RanGreen);
			glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
			glColor3f(rndm,0.8,0.0); // green
			Vector3 c = skin._particles[i3].pos;
			Vec3f na = skin.VerNorms[i3];
			glNormal3f(na.x(),na.y(),na.z());
			glVertex3f(c.x,c.y,c.z);
			
		}
	}
	glEnd();
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glEnableClientState (GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_DOUBLE, sizeof(Particle), &(skin.GetParticleAt(0)->pos.x));
	glNormalPointer(GL_FLOAT, 0, skin.VerNorms);
	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_NORMAL_ARRAY );
	/* draw geometry (vertex arrays) */
	// draw filled triangles
	glColor3f(0.0f, 0.0f, 1.0f);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,Blue);
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	glDrawElements(GL_TRIANGLES , skin.nb_tri*3, GL_UNSIGNED_INT, &skin._faces[0]);
	/* draw geometry (vertex arrays) */
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	glColor3f(0.09f, 0.09f, 0.09f);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,AWHITE);
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );
	glDrawElements(GL_TRIANGLES , skin.nb_tri*3, GL_UNSIGNED_INT, &skin._faces[0]);
	
	// glDisableClientState (GL_NORMAL_ARRAY);
	glDisableClientState (GL_VERTEX_ARRAY);
     glDisable(GL_LIGHTING);
	//glLoadIdentity();
	glPopMatrix();
	//
	//glDisable(GL_LIGHT1);
	glPolygonMode(GL_FRONT_AND_BACK ,GL_FILL);
}
