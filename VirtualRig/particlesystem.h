// codice per simulazione fisica con verlet
// simulation system using Verlet integration
// coded by Marco Fratarcangeli
#ifndef __PARTICLE_SYSTEM__
#define __PARTICLE_SYSTEM__
#define NUM_ITERATIONS 8// numero massimo di iterazioni per soddisfare i vincoli per rilassamente
// max number of iteration to satisfy the constraints
#define TIME_STEP 0.001f
#include <string.h>
#include "particle.h"
#include <GL/glew.h>
#include "constraint.h"
#include "edge.h"
#include "bag.h"
#include "array.h"
#include "triangle.h"
#include <assert.h>
#include <vector>
#include "Pboundingbox.h"
class CSkeleton;
class Triangle;
class VertexParent;
//typedef unordered_set<std::pair<int, int >> MyEdge;
class ParticleSystem
{

	Vector3    m_vGravity;			// define gravity force
	float      m_fTimeStep;			// timestep ( we can update the simulation by this parameter)
	int S_iNbIterations;  // number of iteration in order to satisfy the streach constraint
	int G_iNbIterations;  // number of iteration in order to satisfy the all constraint
	int B_iNbIterations;  // number of iteration in order to satisfy the bone constraint

public:
	void VcgOrient(ParticleSystem & p); // reorder the mesh faces in order to calculate   
	void setBindPose();  // set the T pose
	void CalculateFaceNormals(); // Calculate normals  
	void ComputeNormals(); // different method
	void CalculateVerNormals();
	Vec3f *TriNorms; // pointer of array where we can store normal for each face
	Vec3f *VerNorms; // pointer of array where we can store normal for each vertex
	Vec3f *RestVerNorms; // pointer of array where we can store normal for each vertex
	void Allocate(int nbTris, int nbVert );
	PBoundingBox *boundbox; // define  boundingbox verible
	vector<Particle>		_particles;	// set of particles stored in the particle system
	vector<Particle>		_bindparticles;	// set of particles stored in the rest pose particle system
	std::vector< std::pair<int,Vector3>> triangle;
	// which in this case mesh vertices
	vector<Vector3>			_normals;
	vector<int>			_faces; // skin vertex indices
	vector<int>			_bindfaces; // skin vertex indices
	vector<int>			_tetfaces; // tet indices
	vector<Constraint>		_constraints;	// set of constraints influencing the particles
	//  number of constraint we can define them to control skin 


	set<PEDGE> _edges; // skin edges
	bool solve_types[6];	// what are the constraints to solve / allocate 
	//(which could be stretch ... ect ) 
	vector<int>			_tetras;  // for tetrahedral 
	//vector<Vector3>		_normals; // compute normal
	int nb_tri; 
	int nb_tetera;
	void computeNormals();
	ParticleSystem(); // empty default constructor  
	~ParticleSystem(){
		delete edges;
		edges = NULL;
		delete vertices; 
		vertices = NULL;
		delete triangles;
		triangles = NULL;
		delete boundbox;
		boundbox = NULL;
		delete[] TriNorms;
		delete[] VerNorms;
		delete[] RestVerNorms;
		_particles.clear();
		_constraints.clear();
		_faces.clear();
		_edges.clear();
		_tetras.clear();
	} // destructor 
	// (set the default paramters for stiffness
	void Reset();  // clear all dynamic allocation paramters
	void SetGravity(Vector3 g); // set gravity 	
	void SetTimeStep(float ts); // update time step
	float GetTimeStep(); // get timestep
	void SetNbIterations(int ni); // number of All iteration
	int GetNbIterations(); // get number of All iteration
	void SetSNbIterations(int ni); // number of Streach iteration
	int GetSNbIterations(); // get number of Streach iteration
	void SetBNbIterations(int ni); // number of bone iteration
	int GetBNbIterations(); // get number of bone iteration
	Particle* GetParticleAt(int i); // get particale at specific index 
	Particle* AddParticle(Particle _p); // add particle to particles system
	Constraint * AddConstraint(Constraint _c); // add constraints to the system
	void AddFace(int id[3]); // add face to particles system
	void AddPEdges(int id[3]); // add edges to particles system
	void AddTetra(int id[4]); // which is already done by CGAL Mesh_3 demo
	void TimeStep(); // assigns forces to the particles
	void DefineConstraints();
	void DefineTetConstraints();
	void DefineStrConstraints();
	void DefineStretchConstraints();
	void DefinebendConstraints();
	void DefineVolumeConstraints();
	double CalculateTriArea( Vec3f v1,Vec3f v2,Vec3f v3);
	void DefineSystem(	int nb_vertices, 
		int nb_faces, 
		int nb_edges, 
		vector<Particle> _P,
		vector<int> _F,
		set<PEDGE> _E,
		vector<int> _T);  // define particle system which 
	// should contian faces , vertices , edges
	//void Draw();
	void SetNormals() {}; // set normal which already compute in mesh class
	float stretch_stiffness; // define stiffness for stretch constraints 
	// ( so we could send the desrid value)
	float bending_stiffness; // same for bending and volume
	float volume_stiffness;
	float bone_stiffness;
	Bag<ParticleEdge*> *edges; // edge according to half edge data structure
	Array<EVertex*> *vertices; // array of pointer to store the all vertices
	ParticleEdge* GetEdge(EVertex *a, EVertex *b);
	EVertex* GetVertex(int i);
	int numVertices() const { return vertices->Count(); } // number of vertices
	void AddTriangle(EVertex *a, EVertex *b, EVertex *c); // add triangle by adding three vertices
	void AddTetTriangle(EVertex *a, EVertex *b,EVertex *c,EVertex *d);
	void RemoveTriangle(Triangle *t); // remove triangle by sending the exact triangle
	EVertex* AddVertex(const Vec3f &position); //add vertex to half edge 
	Bag<Triangle*> *triangles; // half edge structure faces
	void DrawLinkBone();
	void CollisionPlane(Vector3 p, Vector3 n); // plane collision
	CSkeleton * ske;
	// skeleton system in order to find the nearest bone and apply constraint
	// consist size of the vertices , faces and teterhderal
	/*static int v_offset;
	static int f_offset;
	static int t_offset;
	static int e_offset;
	static int tri_offset;*/
	static int stretch_constriant;
	static int tet_constriant;
	static int bend_constriant;
	static int bone_constriant;
	// needed to compute the centeriod 
	Vector3 centroid;
	Vector3 bmax, bmin;
	void ComputeCentroid();
	void Draw();
	Vec3f Maxboundingbox();// find max bounding box
	Vec3f Minboundingbox();// find min bounding box

private:
	// in questa funzione si assegnano tutte le forze agenti su ogni particella
	// This function you assign all the forces acting on each particle
	void AccumulateForces();
	void Verlet();	 // Verlet integration for each timestep	
	void SatisfyConstraints(); // iterate till the constraint satisfy 	
	void SatisfyTetConstraints (); // satisfy tet constraint 
	void SatisfyStrConstraints (); // satisfy str constraint 
	void SatisfyBoneConstraints (); // satisfy tet constraint 
};

#endif // __PARTICLE_SYSTEM__                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                      