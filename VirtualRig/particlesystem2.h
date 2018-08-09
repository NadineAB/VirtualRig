// codice per simulazione fisica con verlet
// simulation system using Verlet integration
// coded by Marco Fratarcangeli
#ifndef __PARTICLE_SYSTEM__
#define __PARTICLE_SYSTEM__

#define NUM_ITERATIONS 16		// numero massimo di iterazioni per soddisfare i vincoli per rilassamente
// max number of iteration to satisfy the constraints
#define TIME_STEP 0.01f
#include "particle.h"
#include <GL/glew.h>
#include "constraint.h"
#include "edge.h"
#include "bag.h"
#include "array.h"
#include "triangle.h"
#include <assert.h>
#include <vector>

using namespace std;
class CSkeleton;
class Triangle;
class VertexParent;
class ParticleSystem
{

	Vector3     m_vGravity;			// define gravity force
	float      m_fTimeStep;			// timestep ( we can update the simulation by this parameter)
	int m_iNbIterations;  // number of iteration in order to satisfy the constraint

public:
	vector<Particle>		_particles;	// set of particles stored in the particle system
	// which in this case mesh vertices
	vector<int>			_faces; // skin vertex indices
	vector<Constraint>		_constraints;	// set of constraints influencing the particles
	//  number of constraint we can define them to control skin 
	vector<ParticleEdge>	 _edges; // skin edges
	bool solve_types[6];	// what are the constraints to solve / allocate 
	//(which could be stretch ... ect ) 
	vector<int>			_tetras;  // for tetrad constraint
	vector<Vector3>		_normals; // compute normal
	ParticleSystem(); // empty default constructor  
	~ParticleSystem(){
		delete edges;
		edges = NULL;
		delete vertices; 
		vertices = NULL;
		delete triangles;
		triangles = NULL;} // destructor 
	// (set the default paramters for stiffness
	void Reset();  // clear all dynamic allocation paramters
	void SetGravity(Vector3 g); // set gravity 	
	void SetTimeStep(float ts); // update time step
	float GetTimeStep(); // get timestep
	void SetNbIterations(int ni); // number of iteration
	int GetNbIterations(); // get number of iteration
	Particle* GetParticleAt(int i); // get particale at specific index 
	Particle* AddParticle(Particle _p); // add particle to particles system
	Constraint * AddConstraint(Constraint _c); // add constraints to the system
	void AddFace(int id[3]); // add face to particles system
	void AddPEdges(int id[3]); // add edges to particles system
	void AddTetra(int id[4]); // which is already done by CGAL Mesh_3 demo
	void TimeStep(); // assigns forces to the particles
	void DefineConstraints();
	void DefineSystem(	int nb_vertices, 
		int nb_faces, 
		int nb_edges, 
		vector<Particle> _P,
		vector<int> _F,
		vector<ParticleEdge> _E);  // define particle system which 
	// should contian faces , vertices , edges
	//void Draw();
	void SetNormals() {}; // set normal which already compute in mesh class
	float stretch_stiffness; // define stiffness for stretch constraints 
	// ( so we could send the desrid value)
	float bending_stiffness; // same for bending and volume
	float volume_stiffness;
	Bag<ParticleEdge*> *edges; // edge according to half edge data structure
	Array<EVertex*> *vertices; // array of pointer to store the all vertices
	ParticleEdge* GetEdge(EVertex *a, EVertex *b);
	EVertex* GetVertex(int i);
	int numVertices() const { return vertices->Count(); } // number of vertices
	void AddTriangle(EVertex *a, EVertex *b, EVertex *c); // add triangle by adding three vertices
	void RemoveTriangle(Triangle *t); // remove triangle by sending the exact triangle
	EVertex* AddVertex(const Vec3f &position); //add vertex to half edge 
	Bag<Triangle*> *triangles; // half edge structure faces
	void DrawLinkBone();
	CSkeleton * ske;
	// skeleton system in order to find the nearest bone and apply constraint
private:
	// in questa funzione si assegnano tutte le forze agenti su ogni particella
	// This function you assign all the forces acting on each particle
	void AccumulateForces();
	void Verlet();	 // Verlet integration for each timestep	
	void SatisfyConstraints(); // iterate till the constraint satisfy 	
};

#endif // __PARTICLE_SYSTEM__