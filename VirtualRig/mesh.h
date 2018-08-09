#ifndef CMESH_H
#define CMESH_H
#pragma warning (disable: 4002)
#pragma warning (disable: 4003)
#include "material.h"
#include "particlesystem.h"
#include "particle.h"
#include <wrap/io_trimesh/import.h>
#include <vcg/simplex/face/pos.h>
using namespace std;
using namespace vcg;
class CFace;
class CEdge;
class CVertex;
class CMesh;
class CBone;
/*! 
*\brief:  Mesh Interface ****
which contain the needed component of the mesh include
*vertices, faces and edges.
* coded by Marco Fratarcangeli
=========================================================*/
// UsedTypes is class contian edge, vertex , face  and half edge
struct MyUsedTypes : public UsedTypes<Use<CVertex>::AsVertexType,  // vetex
	Use<CFace>::AsFaceType,
	Use<CEdge>::AsEdgeType>{}; // face
// compositing wanted proprieties
// Vertex Coor Normal
// UVcoord , Vertex adj in the same face
// index for each vertex easly use in 
class CVertex : public vcg::Vertex<MyUsedTypes, vcg::vertex::Coord3f, vcg::vertex::Normal3f, vcg::vertex::VFAdj, vcg::vertex::TexCoord2f, vcg::vertex::Color4b, vcg::vertex::BitFlags>
{
	int index; 
	CoordType tangent;  // tangent
	/* in order to compute tangent we use UV coord or tex coord which we could 
	/ compute using barycenteric
	/ and edge UV / tex coord  */
	CoordType bitangent; // bitangent orthogonal to both the Normal vector 
	// and the chosen Tangent vector biTangent = TXN 
	//( crossproduct tangent and normal )
public:

	/*! 
	* \brief:
	* class interface to assign the bone with specific weight 
	*/
	class CBoneWeight
	{
	public: 
		CBone * bone;	     // the bone object
		float weight;	     // the weight of the i-th bone
		Point3f lcoord;	// coordinates of the vertices in the local frame of the bone
	};

	vector<CBoneWeight> bone_weight;  /* Vector of class  vertex , weight , local coord of the bone
							    /*!
							    /*  \brief: Construct initalize index for vertex with =-1 , zero tangent and bi tangent
							    */
	CVertex()
	{
		index = -1;
		tangent.SetZero();
		bitangent.SetZero();
	}
	// method to assgin averge vertices weight influnce by specific bone
	void NormalizeWeights();
	void PushBoneWeight(const CBoneWeight & bw);

	// index shall be assigned as a optional component
	int & Index()			{	return index;	}

	// tangent shall be assigned as a optional component
	CoordType & Tangent()	{	return tangent;	}
	CoordType & Bitangent()	{	return bitangent;	}
};
/*! 
*\brief:  face Interface 
which contain 
Face vetices that made up the face
Face noraml
Edge tex coord
Adj faces
*/
class CFace   : public vcg::Face<MyUsedTypes, vcg::face::VertexRef, vcg::face::Normal3f, vcg::face::WedgeTexCoord2f, vcg::face::VFAdj, vcg::face::BitFlags, vcg::face::Mark > {};
class IFace   : public vcg::Face<MyUsedTypes, vcg::face::VertexRef, vcg::face::Normal3f, vcg::face::WedgeTexCoord2f, vcg::face::VFAdj, vcg::face::BitFlags, vcg::face::Mark ,vcg::face::Pos > {};
class CEdge   : public vcg::Edge<MyUsedTypes, vcg::edge::EEAdj,vcg::edge::EFAdj, vcg::edge::EVAdj, vcg::edge::VertexRef, vcg::edge::BitFlags, vcg::edge::Mark > {};
/*! 
*\brief:  Mesh Interface ****
which made up tringle faces
*/
class CMesh   : public vcg::tri::TriMesh< std::vector<CVertex>, std::vector<CFace> > 
{
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(filename);
		ar & BOOST_SERIALIZATION_NVP(material);
	}

	void FindMorphMapping();

protected:

public:
	ParticleSystem skin; // define skin in physics sim
	typedef GridStaticPtr<FaceType, ScalarType> SpatialGridType;
	typedef tri::FaceTmark<CMesh> MarkerFace; // selected faces
	CMaterial material;
	string	  filename;     // mesh file
	vector<int> mapping;	// mapping[i] is the index of the vertex inthe morph target corresponding to vertex i
	vector<CMesh*> morphs;	// morph target of type mesh
	Color4f color;
	CMesh();
	~CMesh();
	void DefineParticleSystem();
	string Init();
	void LoadMorphs(const string & dir, const string & key);
	void ApplyMorph(int index);
	// rotation by changing texture coordinates (useful for eye gaze)
	void DrawSkin();
	void DrawWireSkin();
	void DrawSkinError();
	void UpdateSkin();
	ScalarType pitch, opitch;
	ScalarType yaw, oyaw;
	bool is_static;	// if true, the mesh does not move; false, by default
	bool is_transparent;	// if true, the mesh is blended
	bool needs_grid;	// the gris is used to find heat weights and make spatial queries; it is not always needed

};
#endif