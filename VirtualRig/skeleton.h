#ifndef SKELETON_H
#define SKELETON_H
#pragma warning (disable: 4002)
#pragma warning (disable: 4003)
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include "mesh.h"
#include <GL/glew.h>
#include <vcg/space/segment3.h>
#include <vcg/space/distance3.h>
#include <wrap/gl/space.h>
#include <wrap/gl/gl_geometry.h>
#include <vcg/math/matrix33.h>
#include <vcg/space/point3.h>
#include <vcg/space/box3.h>
#include <QString>
using namespace vcg;
using namespace std;

class CMesh;
class CVertex;
/////////////////////////////////////////////////////////////////////////////////////
class CBone
{
	/*! \brief:  Class interface for bone
	* \contain
	* index of the node it could be vertex	
	* index for parent joint 
	* index for child joint
	/*coded by Marco Fratarcangeli
	*************************************************************************
	* what about constraints the DOF 
	* like for each joint value of DOF with max and min to control the joint 
	* rotation 
	* set value() , get vlue () and set maxminvalue ()
	*************************************************************************/
public:
	int node; // index for node itself
	int parent; // index for parent 
	int child; // index for child
	/// Point3f is type of data in vcg
	Point3f lchild; // local coordinate of the child vert (local coordinate bone coordinate)

	vector<CBone*> childs;   // vector of type Cbone represent more than one child
	vector<Point3f> lchilds;	// local coordinate of the children

	int index;	// index of the bone within the skeleton vector
	float rdist;	// rest distance

	//	Point3f p;

	Matrix33<float> L;	// local coordinate frame

	//vector<CVertex*> vp;	// vector of pointers to the vertices handled by this bone
	//vector<float> w;	// weight of the i-th vertex in vp

	//Point3f X()	{		return Point3f(L[0][0], L[1][0], L[2][0]);	}
	//Point3f Y()	{		return Point3f(L[0][1], L[1][1], L[1][2]);	}
	//Point3f Z()	{		return Point3f(L[0][2], L[2][1], L[2][2]);	}
	bool leaf; 
	Point3f x, y, z;		// coordinate axis

	Point3f z_prev;	// z vector at the previous iteration -------------------
	Point3f z_rest;	// z vector at the rest position ------------------------
     CBone ()
	{
	leaf=false;
	}

};

/*! \brief:  Class interface for Skeleton
* links indices between joints represent as Clink
* index for parent joint 
* vector index for childen joint
*/
class CSkeleton
{
public:

	CSkeleton(); 

	// helper struct to build the bones from the file format of pinocchio
	// http://www.mit.edu/~ibaran/autorig/
	class CLink  /// Clink class
	{    
	public:
		int node;  // index for joint
		int parent; // index for parent joint
		vector<int> childs; // indices of joints
	};
	/*! \brief:  FindLink bone between joint
	* Param: node 	is it vertex??? NO its joint
	* try to search if there is have link with any other joint ????
	* 
	*/
	int FindLink(int node)
	{
		for (int i = 0; i < link.size(); i++)
			if (link[i].node == node)
				return i;

		return -1;
	}
	void AssginWeights();
	int FindLink(const Point3f & p); // find link between bone and bone
	void AddLink(const Point3f & p); //add link between bone and bone or joint
	// sent two bones and link between them
	void AddLink(const Point3f & s, const Point3f & d);
	vector<CLink> link; //  vector of links
	vector<CBone> bone; // vector of bones
	int root; // root joint
	string filename;
	bool inited; // if the skeleton initilize or not

	Box3f bbox; // bounding box
	//ParticleSystem phys; // define skin in physics sim
	CMesh * mesh; // object of type Cmesh in order to assgin for each vertex the nearest joint
	int bone_to_show_index;
     void ParticleNearestBone(); // find the nearest bone for each particle
	void AssginParticlesWeights(); // normalize weight
	void StretchBoneConstraint();   // bind the particle to skeleton
	vector<Point3f> vert;		// position of the i-th link 
	vector<Point3f> vert_prev;	// previous position of the i-th link 
	vector<Point3f> vert_rest;	// rest position of the i-th link 
	vector<Point3f> vert_bind;	// bind pose of the i-th link 
	vector<bool>    movable;	// if i-th element is true, the vert[i] can move
	// bone vertex assigment 
	vector<pair<int, int>> vertassignbone;	// which  vertex index and bone index	
	// helpers to rotate just the arm in the mh dataset
	Point3f dir_rot; // bone direction  
	float max_angle_rot; // max angle we can rotate
	// rotate bone depend on specific anagle
	void RotateBone(int index_bone, float angle, Point3f axis_rot);
	void RotateBone(const Matrix33f & mrot, const CBone* pParent, CBone* pBone);
	//bool dist_bone_comp_func_ske(pair<float, CBone*> i, pair<float, CBone*> j);
	void Reset(); // reset based on local coordinate
	void bindpose(); // Bind based on local coordinate
	int Load(); // load male.skel , skel done MIT
	int LoadNew(const QString & filePath);
	int Save(); // save s skel
	void Init(); // init local coordinate and establish linking between joints
	void Assign(CMesh *mesh); // assgin skeleton to mesh
	void Update();
	void VertexNearestBone(); // for each vetex in mesh find the nearest joint
	void InitLocalFrame(); // direction of each bone
	void UpdateLocalFrame(); // update when any rotation happened

	void InitLocalChilds();

	void Draw(bool is_opaque = false);  // draw skeleton

	void IncBoneToShow();
	void DecBoneToShow();

	bool is_visible; // show / hide the skeleton
	float skel_average_distance;

private:
	//	void SetMovable(const CBone & _bone);
	void SetMovable(int i);
};

#endif //__SKELETON