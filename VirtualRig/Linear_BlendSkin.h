#ifndef LINEAR_BLENDSKIN_H
#define LINEAR_BLENDSKIN_H
#pragma warning (disable: 4002)
#pragma warning (disable: 4003)
#include "mesh_gl.h"
using namespace std;
using namespace vcg;
/*! 
* \brief:
* Base on class CMesh to render and load model
* and build the loaded skeleton
* including assgin weight  based on Linear BLend Skin
* coded by Marco Fratarcangeli
*/
class CMeshGL;
class CSkeleton;
class Linear_BlendSkin
{

public:
	vector<CMeshGL*> LinerSkinmeshes; // vector of type meshes // eyes load as different mesh so its not combine object
	CSkeleton * Linearskeleton; //  object of type skeleton which is mel.skel
	Linear_BlendSkin(); // constructor
	void ReadyUpadteSkel();
	void FinalizeWeights(CMesh *SmoothWeights);
	void LBSUpdate();	// update position of the vertices according to the configuration of the bones
	~Linear_BlendSkin();
};

#endif // _LINEAR_BLENDSKIN_H__