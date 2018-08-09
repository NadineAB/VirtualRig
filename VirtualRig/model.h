#ifndef MODEL_H
#define MODEL_H
#pragma warning (disable: 4002)
#pragma warning (disable: 4003)
#include "mesh_gl.h"
using namespace std;
using namespace vcg;
/*! 
* \brief:
* Base on class CMesh to render and load model
* and build the loaded skeleton
* including assgin weight
* coded by Marco Fratarcangeli
*/
class CMeshGL;
class CSkeleton;
class CModel
{
	friend class boost::serialization::access;
	template<class Archive>
	void serialize(Archive & ar, const unsigned int version)
	{
		// serialize base class information
		ar & BOOST_SERIALIZATION_NVP(name);
		ar & BOOST_SERIALIZATION_NVP(meshes);
	}

	vector<Point3f> dpoints; // point
	vector<pair<Point3f, Point3f>> dlines; // line between two point

public:
	int max_distance;
	int static count;
	string bvhname; // load bvh file
	vector<pair<int, int>> parassignbone;	// which  vertex index and bone index	
	string name;   // model file name in disk which nh/male.xml 
	string anim_dir; // directory of the bvh files
	vector<CMeshGL*> meshes; // vector of type meshes // eyes load as different mesh so its not combine object
	CSkeleton * skeleton; //  object of type skeleton which is mel.skel
	CModel(); // constructor
	void BindSkin();
	void LinearSkin(CMesh* skin);
	string Init();
	void Update(bool anim, int strstiff );	// update position of the vertices according to the configuration of the bones
	//void Update();	// update position of the vertices according to the configuration of the bones
	void ReadyUpade();
	void Saveobj(); // in order to bump .obj file on the desk
	void Draw(bool error, bool render);
	~CModel();
	void FinalizeWeights(CMesh *);
	void LinearSmoothSkin(CMesh *);
	void averageDistance(CMesh *SmoothWeights);
	int loadWeight(CMesh * SmoothWeights);
	void updateLinearSkin(int stiff,int str,int stiffnesstr_prevvalue);
	//	void AddMesh(CMeshGL * pM);
	int GetNbMorphs();
	void ApplyMorph(int index);
	bool lines_visible; // bvh linking lines to skel joint
	float total_distance; // total distance
	float average_distance; // average distance
};

#endif // __MODEL__