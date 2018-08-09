//#define NUM_ITERATIONS 4	// max number of iteration to satisfy the constraints	
//#define TIME_STEP 0.005f
#include "skeleton.h"
#include "particle.h"
#include "constraint.h"
#include "particlesystem.h"
#include "base_types.h"
#include <vector>
#include <map>
using namespace vcg;
using namespace std;
#define INITIAL_VERTEX 10000000
#define INITIAL_EDGE 10000000
#define INITIAL_TRIANGLE 10000000
/////// define profile and profile node to estimate code proformance 
#include "profile.h"
/*#include<vcg/complex/complex.h>
#include<vcg/complex/algorithms/create/platonic.h>*/
#define USE_PROFILE		// undef to disable profiling
class MyEdge;
class MyFace;
class MyVertex;
struct MyPUsedTypes : public UsedTypes<	Use<MyVertex>   ::AsVertexType,
                                        Use<MyEdge>     ::AsEdgeType,
                                        Use<MyFace>     ::AsFaceType>{};

class MyVertex  : public Vertex<MyPUsedTypes,  vertex::Coord3f, vertex::Normal3f, vertex::VFAdj, vertex::Qualityf, vertex::Color4b, vertex::BitFlags  >
{
public:
	int index;
};
class MyFace    : public Face< MyPUsedTypes,   face::VertexRef, face::Normal3f, face::BitFlags, face::VFAdj, face::FFAdj > {};
class MyEdge    : public Edge< MyPUsedTypes, edge::VertexRef, edge::BitFlags>{};
class MyMesh    : public tri::TriMesh< vector<MyVertex>, vector<MyEdge>, vector<MyFace>   > {};
////////////////////////////////////////////////////////////////////
//default value of the particle system
int ParticleSystem :: bend_constriant=0;
int ParticleSystem ::bone_constriant=0;
int ParticleSystem ::stretch_constriant=0;
int ParticleSystem ::tet_constriant=0;
void ParticleSystem ::VcgOrient(ParticleSystem & p)
{
	    // cout<< "\n--------Orient Particle1--------------\n";
		MyMesh outMesh;
		outMesh.Clear();
		vcg::tri::Allocator<MyMesh>::AddVertices(outMesh, p._particles.size());
		for(unsigned int i=0;i< p._particles.size();++i)
		outMesh.vert[i].P()=vcg::Point3f(p._particles[i].pos.x,p._particles[i].pos.y,p._particles[i].pos.z);
		for(size_t i = 0; i < outMesh.vert.size(); ++i)
		outMesh.vert[i].index = i;
		vcg::tri::Allocator<MyMesh>::AddFaces(outMesh, p.nb_tri);
		for(size_t i = 0; i < p.nb_tri; i ++)
		{
			//cout<< "\n--------Orient Particle4--------------\n";
			 outMesh.face[i].V(0)=&(outMesh.vert[p._faces[i * 3]]);
			 outMesh.face[i].V(1)=&(outMesh.vert[p._faces[i * 3 + 1]]);
			 outMesh.face[i].V(2)=&(outMesh.vert[p._faces[i * 3 + 2]]);
		}
		
		bool oriented,orientable;
		//cout<< "\n--------Orient Particle5--------------\n";
		tri::UpdateTopology<MyMesh>::FaceFace(outMesh);
		//cout<< "\n--------Orient Particle5--------------\n";
		tri::Clean<MyMesh>::OrientCoherentlyMesh(outMesh,oriented,orientable);
		//assert(orientable);
		tri::Clean<MyMesh>::FlipMesh(outMesh);
		
		p._faces.clear();
		//cout<< "\n--------Orient Particle6--------------\n";
		for (size_t i = 0; i < outMesh.face.size(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
                 	p._faces.push_back(outMesh.face[i].V(j)->index);
			}
		}
		
}
ParticleSystem::ParticleSystem()
{
	_particles.reserve(10000);
	_constraints.reserve(10000);
	_faces.reserve(10000);
	_tetras.reserve(10000);
	_bindparticles.reserve(10000);
	_bindfaces.reserve(10000);
	//_edges.re
	m_fTimeStep = TIME_STEP;
	S_iNbIterations = NUM_ITERATIONS; 
	G_iNbIterations = NUM_ITERATIONS;  
	B_iNbIterations = NUM_ITERATIONS;
	stretch_stiffness = 1.f;
	bending_stiffness = 1.f;
	volume_stiffness = 1.f;
	edges = new Bag<ParticleEdge*>(INITIAL_EDGE,ParticleEdge::extract_func);
	vertices = new Array<EVertex*>(INITIAL_VERTEX);
	triangles = new Bag<Triangle*>(INITIAL_TRIANGLE,Triangle::extract_func);
	boundbox = NULL;
	TriNorms = NULL;
	RestVerNorms=NULL;
	VerNorms= NULL;
	for ( int i = 0; i < nb_tri; i++ )  // for loop the number of triangles 
	{
		TriNorms[i].Set(0.0,0.0,0.0);
	}
	for(int i = 0; i < _particles.size(); i++)
	{
		VerNorms[i].Set(0.0,0.0,0.0);
		RestVerNorms[i].Set(0.0,0.0,0.0);
	}
}
// reset the state of the particel system
void ParticleSystem::Reset()
{    
	PROFILE_SAMPLE("ResetParticle");
	_particles.clear();
	_constraints.clear();
	_faces.clear();
	_edges.clear();
	_tetras.clear();
	_bindparticles.clear();
	_bindfaces.clear();
}
void ParticleSystem::Allocate(int nbTris, int nbVert )
{
	TriNorms = new Vec3f[ nbTris + 1 ];
	VerNorms = new Vec3f[ nbVert + 1 ];
	RestVerNorms= new Vec3f[ nbVert + 1 ];
}
// set timestep value
void ParticleSystem :: SetTimeStep(float ts)		
{
	if (ts > 0)
		m_fTimeStep = ts;
}
float ParticleSystem ::GetTimeStep() {return m_fTimeStep;}   // return the timestep
// set and get for number of iteration for each type of constraint 
void ParticleSystem::SetNbIterations(int ni)	
{
	if (G_iNbIterations > 0)
		G_iNbIterations = ni;
}
int ParticleSystem::GetNbIterations() {return G_iNbIterations;}

Particle* ParticleSystem ::GetParticleAt(int i)
{
	assert(i < (int)_particles.size());
	return &_particles[i];
}
void ParticleSystem::SetSNbIterations(int ni)	
{
	if (S_iNbIterations > 0)
		S_iNbIterations = ni;
}
int ParticleSystem::GetSNbIterations() {return S_iNbIterations;}
void ParticleSystem::SetBNbIterations(int ni)	
{
	if (B_iNbIterations > 0)
		B_iNbIterations = ni;
}
int ParticleSystem::GetBNbIterations() {return B_iNbIterations;}
// add particle in particle system
/**************************************************************************** 
///////////////////////////// Performance Profiling \\\\\\\\\\\\\\\\\\\\\\\\\  
c.1: ParticleSystem:: AddParticle
/*****************************************************************************/
Particle* ParticleSystem:: AddParticle(Particle _p)
{ 
	PROFILE_SAMPLE("AddParticle");
	Vector3 ver;
	ver.x=_p.pos.x;
	ver.y=_p.pos.y;
	ver.z=_p.pos.z;
	Vec3f evert(ver.x,ver.y,ver.z);
	// Add Vertex no loop complexity O(4)
	AddVertex(evert);// add it as vertex in edge system JUST BAD WAY to get border
	_p.index = _particles.size();
	//cout << "particle " << _p.index;
	_particles.push_back(_p);
	_bindparticles.push_back(_p);
	return &(_particles.back());
}
// add constraints
///////////////////////////// Performance Profiling \\\\\\\\\\\\\\\\\\\\\\\\\  
// update the physical system
// advance the simulation of one time step
void ParticleSystem::TimeStep()
{    
	PROFILE_SAMPLE("AddParticle");
     AccumulateForces();
	Verlet();					// numerical integration with Verlet (the paper does it better)
	SatisfyConstraints();		// solve the constraints defined on this particle system
	////cout << "\n \n iteration " << GetNbIterations(); 
}
// add Faces
//c.1: ParticleSystem:: AddFace complexity O(10) for each face
/*****************************************************************************/
void ParticleSystem ::AddFace(int id[3])
{
	//cout << f_offset;
	PROFILE_SAMPLE("AddFace");
	int id_face = _faces.size() / 3 ;
	for (int j = 0; j < 3; j++)
	{
		_faces.push_back(id[j]);
		_bindfaces.push_back(id[j]);
		Particle & p = *(GetParticleAt(id[j]));
		p.faces.insert(id_face);
		p.neighs.insert(id[j]);
	}
	// append memory space for the normal of this face
	_normals.push_back(Vector3());
}
///////////////////////////// Performance Profiling \\\\\\\\\\\\\\\\\\\\\\\\\  
//c.2: ParticleSystem:: add edges
/*****************************************************************************/
void ParticleSystem ::AddPEdges(int id[3])
{
	PROFILE_SAMPLE("AddPEdges");
	// for each face index loop of complexity O(42
	for (int i = 0; i < 3; i++)
	{
		int i0 = id[i];
		int i1 = id[(i + 1) % 3];
		if (i0 > i1)
			swap(i0, i1);
		//EVertex *va = GetVertex(i0); // complexity O(3)
		//EVertex *vb = GetVertex(i1); // complexity O(3)
		//ParticleEdge *ab = GetEdge(va,vb); // GetEdge complexity O(2) 
		PEDGE e;
		e.setPEDGE(i0, i1);
		_edges.insert(e);
		set<PEDGE>::iterator it = _edges.find(e);
		assert(it != _edges.end());
	}
}
void ParticleSystem::AddTetra(int id[4])
{
	int id_tetra = _tetras.size() / 4;
	for (int j = 0; j < 4; j++)
	{
		_tetras.push_back(id[j]);
		Particle & p = *(GetParticleAt(id[j]));
		p.tetras.insert(id_tetra);
		int i0 = id[j];
		int i1 = id[(j + 1) % 4];

		if (i0 > i1)
			swap(i0, i1);
		PEDGE e;
		e.setPEDGE(i0, i1);
	     _edges.insert(e);
	}

}
// set gravity O(1)
void ParticleSystem::SetGravity(Vector3 g)
{    PROFILE_SAMPLE("SetGravity");
m_vGravity = g;
}
void ParticleSystem::AccumulateForces()
{
	PROFILE_SAMPLE("AccumulateForces");
	/*_particles[0].isFixed=true;
	_particles[1].isFixed=true;
	_particles[2].isFixed=true;
	_particles[3].isFixed=true;*/
	for(int i=0; i<_particles.size(); i++) 
	{
		if(!_particles[i].isFixed)
		{
			_particles[i].acc=m_vGravity;
			//	cout << _particles[i].acc.y;
		}
	}
}
///////////////////////////// Performance Profiling \\\\\\\\\\\\\\\\\\\\\\\\\  
//d: ParticleSystem:: Define System complexity O(3)
/*****************************************************************************/
/**************************************************************************** 
///////////////////////////// Performance Profiling \\\\\\\\\\\\\\\\\\\\\\\\\  
1. FIRST LOAD 3D MESH
c. CMesh::DefineParticleSystem ( add particle, add faces and add edges)
c.1: ParticleSystem:: AddParticle complexity O(12*n) n vertices
c.2: ParticleSystem:: AddTriangle O(n*38+3) n here number of face
c.3: ParticleSystem:: AddPEdges  O(n*42+3) n here number of face
c.4: ParticleSystem:: AddFace O(n*10+3) n here number of face
d.ParticleSystem:: DefineParticleSystem O(3) 
d.1 ParticleSystem:: DefineConstraints O(14*m +45*m)
f. ParticleSystem ::SetGravity complexity O(1)
/*****************************************************************************/
void ParticleSystem ::DefineSystem(
	int nb_vertices, 
	int nb_faces, 
	int nb_edges, 
	vector<Particle> _p,
	vector<int> _f ,
	set<PEDGE> _e,
	vector<int> _t)
{
	PROFILE_SAMPLE("DefineSystem");
	Reset();
	//cout<< "\n Define Particle System" ;
	_particles =_p;
	_faces=_f;
	_edges=_e;
	_tetras=_t;
	_bindfaces=_f;
	

	for(int  i = 0; i < _particles.size(); i ++)
		{
			//cout<< "\n--------Orient Particle4--------------\n";
			_particles[i].Pstretch_stiffness=1.0/*stretch_stiffness*/;
			_particles[i].Pstet_stiffness=volume_stiffness;
			_particles[i].Pbone_stiffness=bone_stiffness;
		}
	_bindparticles=_p;	
	/////////////// Define Constraint ////////////////
	//DefineConstraints();
	// define stretch constraint
	/*Constraint c(STRETCH, (p0.pos - p1.pos).Length(), stretch_stiffness);
	c.ref_parts.push_back(&p0);
	c.ref_parts.push_back(&p1);
	AddConstraint(c);*
	if (edge_indices[i][3] == -1) // this is a border edge, so skip it
	continue;
	Particle & p2 = *(GetParticleAt(edge_indices[i][2]));
	Particle & p3 = *(GetParticleAt(edge_indices[i][3]));
	// translate everything in _p1
	Vector3 _pos0 = p0.pos;
	Vector3 _pos1 = p1.pos - _pos0;
	Vector3 _pos2 = p2.pos - _pos0;
	Vector3 _pos3 = p3.pos - _pos0;
	// oriented normals
	Vector3  & n0 = (_pos1 ^ _pos2).Normalize();
	Vector3  & n1 = (_pos1 ^ _pos3).Normalize();;
	// rest angle
	double d = n0 * n1;
	double rest_angle = acosf((double)d);
	{
	// define bending constraint
	/*Constraint c(BENDING, rest_angle, bending_stiffness);
	c.ref_parts.push_back(&p0);
	c.ref_parts.push_back(&p1);
	c.ref_parts.push_back(&p2);
	c.ref_parts.push_back(&p3);
	AddConstraint(c);*/
	//}
	//}
	//{
	// define volume constraint
	/*Constraint c(VOLUME, -1, volume_stiffness);
	for (int i = 0; i < nb_vertices; i++)
	c.ref_parts.push_back(GetParticleAt(i));
	for (int i = 0; i < nb_faces; i++)
	c.ref_faces.push_back( &(face_indices[i][0]) );
	c.SetRestVolume();
	volume_constraint = AddConstraint(c);*/
	//}
}
void ParticleSystem::Verlet()
{
	PROFILE_SAMPLE("ParticleVerlet");
	vector<Particle>::iterator pIt;
	for(pIt = _particles.begin(); pIt != _particles.end(); pIt++) 
		if(!pIt->isFixed)
		{
			Vector3& x = pIt->pos;
			Vector3  temp = x;
			Vector3& oldx = pIt->restPos;
			Vector3& a = pIt->acc;
			x += x -oldx + a * m_fTimeStep * m_fTimeStep ;
			// postion+=postion + vel * dt
			oldx = temp;
		}
}		
void ParticleSystem::SatisfyConstraints() 
{   // 24
	//cout << _constraints.size();
	PROFILE_SAMPLE("SatisfyConstraints");
	for(int j = 0; j <4; j++) 
	{	
		vector<Constraint>::iterator cIt;
		for(cIt = _constraints.begin(); cIt != _constraints.end(); cIt++) 
		{
			//cout <<cIt->rest_value; 
			cIt->Solve(GAUSS_SEIDEL_RELAX);
		}
	}
	//cout << "satisfy constriants ";
	//SatisfyBoneConstraints();
	//SatisfyStrConstraints();
	//SatisfyTetConstraints();
	

}
// satisfy teterhadral volume with max number of iteration
void ParticleSystem::SatisfyTetConstraints() 
{
	//cout << _constraints.size();
	// was 3
	PROFILE_SAMPLE("SatisfyConstraints");
	for(int j = 0; j < 12; j++) 
	{	
		vector<Constraint>::iterator cIt;
		for(cIt = _constraints.begin(); cIt != _constraints.end(); cIt++) 
		{
			if(cIt->type==TETRA_VOLUME)
			cIt->Solve(GAUSS_SEIDEL_RELAX);
		}
	}
	//cout << "satisfy constriants ";
}
// satisfy stretch volume with max number of iteration
void ParticleSystem::SatisfyStrConstraints() 
{
	//cout << _constraints.size();
	PROFILE_SAMPLE("SatisfyConstraints");
	for(int j = 0; j < 100; j++) 
	{	
		vector<Constraint>::iterator cIt;
		for(cIt = _constraints.begin(); cIt != _constraints.end(); cIt++) 
		{
			if(cIt->type==STRETCH)
			cIt->Solve(GAUSS_SEIDEL_RELAX);     
		}
	}
	//cout << "satisfy constriants ";
}
// satisfy bone volume with max number of iteration
void ParticleSystem::SatisfyBoneConstraints() 
{
	//cout << _constraints.size();
	PROFILE_SAMPLE("SatisfyConstraints");
	for(int j = 0; j <1/*B_iNbIterations*/; j++) 
	{	
		vector<Constraint>::iterator cIt;
		for(cIt = _constraints.begin(); cIt != _constraints.end(); cIt++) 
		{
			if(cIt->type==BONESTRETCH)
			cIt->Solve(GAUSS_SEIDEL_RELAX);
		}
	}
	//cout << "satisfy constriants ";
}
// get index vertices of the edge comlexity O(2)
ParticleEdge* ParticleSystem ::GetEdge(EVertex *a, EVertex *b) 
{
	assert (edges != NULL);
	return edges->Get(a->getIndex(),b->getIndex());
}
// add vertex complexity O(3 more or less depend on ADD)
EVertex* ParticleSystem :: AddVertex(const Vec3f &position) 
{
	int index =_particles.size();
	//cout << "\n in vertice " << index;
	EVertex *v = new EVertex(index, position);
	vertices->Add(v);
	if (boundbox == NULL) 
		boundbox = new PBoundingBox(position,position);
	// becouse we have just one vertex
	else 
		boundbox->Extend(position);
	return v;
}
// get vertex complexity O(3)
EVertex* ParticleSystem :: GetVertex(int i)  
{
	// is less than zero and greater than number of vertices exist
	EVertex *v = (*vertices)[i];// define vertex by the index
	assert (v != NULL);
	return v;
}
///////////////////////////// Performance Profiling \\\\\\\\\\\\\\\\\\\\\\\\\  
//c.3: ParticleSystem:: add tringle complexity O(38)
/*****************************************************************************/
void ParticleSystem::AddTriangle(EVertex *a, EVertex *b, EVertex *c)
{
	// set the vertices indeces
	// create the triangle
	Triangle *t = new Triangle(); // have three vertices

	// create the edges
	ParticleEdge *ea = new ParticleEdge(a,t); // create edge between the vertices
	ParticleEdge *eb = new ParticleEdge(b,t);
	ParticleEdge *ec = new ParticleEdge(c,t);

	// point the triangle to one of its edges
	t->setEdge(ea);  // complexity O(2)

	// connect the edges to each other complexity O(9)
	ea->setNext(ec);
	eb->setNext(ea);
	ec->setNext(eb);

	// add them to the master list complexity O(6)
	edges->Add(ea);
	edges->Add(eb);
	edges->Add(ec);

	// connect up with opposite edges (if they exist) 
	// complexity O(18)
	ParticleEdge *ea_op = GetEdge((*ea)[1],(*ea)[0]);
	ParticleEdge *eb_op = GetEdge((*eb)[1],(*eb)[0]);
	ParticleEdge *ec_op = GetEdge((*ec)[1],(*ec)[0]);  
	if (ea_op != NULL) { ea_op->setOpposite(ea); }
	if (eb_op != NULL) { eb_op->setOpposite(eb); }
	if (ec_op != NULL) { ec_op->setOpposite(ec); }


	// add the triangle to the master list complxity O(tringle number*7)
	triangles->AddNoDuplicates(t); 
	//cout << " \n " << triangles->Count();
}
void ParticleSystem::AddTetTriangle(EVertex *a, EVertex *b,
	EVertex *c,EVertex *d)
{
	// set the vertices indeces
	// create the triangle
	Triangle *t1 = new Triangle(); // have three vertices
	// create the edges
	ParticleEdge *ea = new ParticleEdge(a,t1); // create edge between the vertices
	ParticleEdge *eb = new ParticleEdge(b,t1);
	ParticleEdge *ec = new ParticleEdge(c,t1);
	// point the triangle to one of its edges
	t1->setEdge(ea);  // complexity O(2)
	// connect the edges to each other complexity O(9)
	ea->setNext(ec);
	eb->setNext(ea);
	ec->setNext(eb);
	edges->AddNoDuplicates(ea);
	edges->AddNoDuplicates(eb);
	edges->AddNoDuplicates(ec);
	triangles->AddNoDuplicates(t1);  //first triangle
	///////////////////////////////////////////////////////////////
	Triangle *t2 = new Triangle(); // have three vertices
	ParticleEdge *eed = new ParticleEdge(d,t2);
	//ParticleEdge *eea = new ParticleEdge(a,t2); // create edge between the vertices
	ParticleEdge *eec = new ParticleEdge(c,t2);
	t2->setEdge(ea);
	ea->setNext(eed);
	eed->setNext(eec);
	eec->setNext(ea);
	edges->AddNoDuplicates(eed);
	//	edges->Add(ea);
	edges->AddNoDuplicates(eec);
	// connect up with opposite edges (if they exist) 
	// complexity O(18)
	triangles->AddNoDuplicates(t2);  //second triangle
	///////////////////////////////////////////////////////////////
	Triangle *t3 = new Triangle(); // have three vertices
	//ParticleEdge *eeea = new ParticleEdge(a,t3);
	ParticleEdge *eedb = new ParticleEdge(d,t3); // create edge between the vertices
	//ParticleEdge *eeeb = new ParticleEdge(b,t3);
	t3->setEdge(eedb);
	eedb->setNext(ea);
	ea->setNext(eb);
	eb->setNext(eedb);
	edges->AddNoDuplicates(eedb);
	//edges->Add(eeea);
	//edges->Add(eeeb);
	//edges->Add(eeed);
	// connect up with opposite edges (if they exist) 
	// complexity O(18)
	// connect up with opposite edges (if they exist) 
	// complexity O(18)
	triangles->AddNoDuplicates(t3);  //thrid triangle
	///////////////////////////////////////////////////////////////
	Triangle *t4= new Triangle(); // have three vertices
	//ParticleEdge *eeeec = new ParticleEdge(c,t4);
	//ParticleEdge *eeeed = new ParticleEdge(d,t4); // create edge between the vertices
	//ParticleEdge *eeeeb = new ParticleEdge(b,t4);
	t4->setEdge(ec);
	ec->setNext(eb);
	eb->setNext(eedb);
	eed->setNext(eec);
	//edges->Add(eeeed);
	//edges->Add(eeeec);
	//edges->Add(eeeeb);
	triangles->AddNoDuplicates(t4); 
	// connect up with opposite edges (if they exist) 
	// complexity O(18)
	/*ParticleEdge *ce_op = GetEdge((*ce)[1],(*ce)[0]);
	ParticleEdge *de_op = GetEdge((*de)[1],(*de)[0]);  
	if (ce_op != NULL) { ce_op->setOpposite(ce); }
	if (ec_op != NULL) { ec_op->setOpposite(ec); }
	if (de_op != NULL) { de_op->setOpposite(de); }
	//thrid triangle*/
	///////////////////////////////////////////////////////////////
	/*edges->Add(ea);
	edges->Add(eb);
	edges->Add(ec);
	edges->Add(ed);
	edges->Add(ce);
	edges->Add(de);*/
	// six edges 
}
void ParticleSystem::RemoveTriangle(Triangle *t) 
{

	ParticleEdge *ea = t->getEdge();
	ParticleEdge *eb = ea->getNext();
	ParticleEdge *ec = eb->getNext();
	assert (ec->getNext() == ea); // back to the same edge
	// remove elements from master lists
	edges->Remove(ea);
	edges->Remove(eb);
	edges->Remove(ec);
	triangles->Remove(t);
	// clean up memory
	delete ea;
	delete eb;
	delete ec;
	delete t;
}
// add new constraint to the particle system complexity O(2)
Constraint* ParticleSystem :: AddConstraint(Constraint _c)
{    
	PROFILE_SAMPLE("AddConstraint");
	_constraints.push_back(_c);
	//cout << " addddddddddddddddddddddd";
	return &(_constraints.back());
}
///////////////////////////// Performance Profiling \\\\\\\\\\\\\\\\\\\\\\\\\  
//d.1: ParticleSystem:: Define constraint 
// for STRETCH O(14*m) m number edges
// for BENDING O(45*m) m umber of edges
/*****************************************************************************/
void ParticleSystem ::DefineConstraints()
{
	//int qq=1;
	PROFILE_SAMPLE("DefineConstraints");
	// for loop for all edges
	Iterator<ParticleEdge*> *iter1 = edges->StartIteration();
	while (ParticleEdge *e = iter1->GetNext()) 
	{
		/*if(qq<2)
		{*/
		if (solve_types[STRETCH])
		{
			int i0 = (*e)[0]->getIndex();
			int i1 = (*e)[1]->getIndex();
			Particle & p0 = _particles[i0];
			Particle & p1 = _particles[i1];
			cout << " stretchhhhhhhhhh " << p0.acc.y;
			//Vec3f a = (*e)[0]->get();
			//Vec3f b = (*e)[1]->get();
			//Particle temp0(a.x(),a.y(),a.z());
			//Particle *sou0 = &temp0;
			//Particle & p0=*sou0;
			//Particle temp1(b.x(),b.y(),b.z());
			//Particle *des1 = &temp1;
			//Particle & p1=*des1;

			// define stretch constraints
			if ((stretch_stiffness > 0) && solve_types[STRETCH])
			{
				float rest =(p0.pos- p1.pos).Length();
				Constraint c(STRETCH, (p0.pos - p1.pos).Length(),stretch_stiffness);
				c.ref_parts.push_back(&p0);
				c.ref_parts.push_back(&p1);
				AddConstraint(c);
				/*qq++;
				}*/
			}
		}
	}

	/*stretch_constriant=_constraints.size() - bone_constriant ;
	cout << "\n Number of Constraints for Stretch: " << _constraints.size() << endl;
	edges->EndIteration(iter1);
	// define bending constraints whereever there are no boundary
	// when e->getOpposite()== NULL this is a border edge, so skip it
	// bending constraints apply in the Triangle
	Iterator<Triangle*> *iter2 = triangles->StartIteration();
	while (Triangle *t = iter2->GetNext())
	{
	if ((*t).getEdge()->getOpposite()!=NULL
	&& (*t).getEdge()!=NULL
	&& (*t).getEdge()->getOpposite() !=NULL
	&& (*t).getEdge()->getNext()->getOpposite()!=NULL )
	{
	ParticleEdge *op=(*t).getEdge()->getOpposite()->getNext();
	ParticleEdge *next=(*t).getEdge();
	ParticleEdge *nextnext=(next)->getNext();
	if ((op->getOpposite()!= NULL) 
	&& (next->getOpposite() != NULL)
	&& (nextnext->getOpposite() != NULL)
	&& (bending_stiffness > 0)
	&& solve_types[BENDING])
	{
	int a = (*t)[0]->getIndex();
	int b = (*t)[1]->getIndex();
	int c = (*t)[2]->getIndex();
	int m =(*op)[0]->getIndex();
	Particle & p0 = _particles[a];
	Particle & p1 = _particles[b];
	Particle & p2 = _particles[c];
	Particle & p3 = _particles[m];
	// translate everything in _p1
	Vector3 _pos0 = p0.pos;
	Vector3 _pos1 = p1.pos - _pos0;
	Vector3 _pos2 = p2.pos - _pos0;
	Vector3 _pos3 = p3.pos - _pos0;
	// oriented normals
	Vector3  & n0 = (_pos1 ^ _pos2).Normalize();
	Vector3  & n1 = (_pos1 ^ _pos3).Normalize();;
	// rest angle
	float d = n0 * n1;
	if (d > 1) d = 1;	// prevent numerical drift
	if (d < -1) d = -1;
	float rest_angle = acosf(d);
	// define the bending constraint
	Constraint bc(BENDING, rest_angle);
	bc.ref_parts.push_back(&p0);
	bc.ref_parts.push_back(&p1);
	bc.ref_parts.push_back(&p2);
	bc.ref_parts.push_back(&p3);
	AddConstraint(bc);
	}
	}
	}
	triangles->EndIteration(iter2);
	bend_constriant=_constraints.size() -stretch_constriant;
	cout << "\n Number of Constraints for bend: " << bend_constriant << endl;
	// Define bone constraints which bend the skin vertices to the bone */

}

void ParticleSystem ::DefineStretchConstraints()
{
	set<PEDGE>::iterator it = _edges.begin();
	for (it = _edges.begin(); it!=_edges.end(); ++it)
	{
		Particle & p0 = *(GetParticleAt(it->pedge.first));
		Particle & p1 = *(GetParticleAt(it->pedge.second));
		// define stretch constraints)
		if (stretch_stiffness > 0)
		{
			//if ((p0.pos - p1.pos).Length() > d_min)
			//	continue;
			Constraint c(STRETCH, (p0.pos - p1.pos).Length(), stretch_stiffness);
			c.ref_parts.push_back(&p0);
			c.ref_parts.push_back(&p1);
			AddConstraint(c);
		}

	}
	stretch_constriant=_constraints.size() - bone_constriant ;
	cout << "\n Number of Constraints for Stretch: " << stretch_constriant << endl;

}
// using half-edge data structure 
void ParticleSystem:: DefineStrConstraints()
{
	//PROFILE_SAMPLE("DefineConstraints");
	// for loop for all edges
	Iterator<ParticleEdge*> *iter1 = edges->StartIteration();
	while (ParticleEdge *e = iter1->GetNext()) 
	{


		int i0 = (*e)[0]->getIndex();
		int i1 = (*e)[1]->getIndex();
		Particle & p0 = _particles[i0];
		Particle & p1 = _particles[i1];
		//cout<< " " << i0 << " " << i1 << endl;
		//Vec3f a = (*e)[0]->get();
		//Vec3f b = (*e)[1]->get();
		//Particle temp0(a.x(),a.y(),a.z());
		//Particle *sou0 = &temp0;
		//Particle & p0=*sou0;
		//Particle temp1(b.x(),b.y(),b.z());
		//Particle *des1 = &temp1;
		//Particle & p1=*des1;
		// define stretch constraints
		if ((stretch_stiffness > 0))
		{
			float rest =(p0.pos- p1.pos).Length();
			Constraint c(STRETCH, (p0.pos - p1.pos).Length(),stretch_stiffness);
			c.ref_parts.push_back(&_particles[i0]);
			c.ref_parts.push_back(&_particles[i1]);
			AddConstraint(c);
		}
	}
	edges->EndIteration(iter1);
	stretch_constriant=_constraints.size() - bone_constriant ;
	cout << "\n Number of Constraints for Stretch: " << stretch_constriant << endl;
}
void ParticleSystem::DefinebendConstraints()
{

	// define bending constraints whereever there are no boundary
	// when e->getOpposite()== NULL this is a border edge, so skip it
	// bending constraints apply in the Triangle
	Iterator<Triangle*> *iter2 = triangles->StartIteration();
	while (Triangle *t = iter2->GetNext())
	{
		if ((*t).getEdge()->getOpposite()!=NULL
			&& (*t).getEdge()!=NULL
			&& (*t).getEdge()->getOpposite() !=NULL
			&& (*t).getEdge()->getNext()->getOpposite()!=NULL )
		{
			ParticleEdge *op=(*t).getEdge()->getOpposite()->getNext();
			ParticleEdge *next=(*t).getEdge();
			ParticleEdge *nextnext=(next)->getNext();
			if ((op->getOpposite()!= NULL) 
				&& (next->getOpposite() != NULL)
				&& (nextnext->getOpposite() != NULL)
				&& (bending_stiffness > 0)
				&& solve_types[BENDING])
			{
				int a = (*t)[0]->getIndex();
				int b = (*t)[1]->getIndex();
				int c = (*t)[2]->getIndex();
				int m =(*op)[0]->getIndex();
				if (a==b || a==c || a==m ||
					b==c || b==m || c==m )
				{
				}
				else
				{
					Particle & p0 = _particles[a];
					Particle & p1 = _particles[b];
					Particle & p2 = _particles[c];
					Particle & p3 = _particles[m];
					// translate everything in _p1
					Vector3 _pos0 = p0.pos;
					Vector3 _pos1 = p1.pos - _pos0;
					Vector3 _pos2 = p2.pos - _pos0;
					Vector3 _pos3 = p3.pos - _pos0;
					// oriented normals
					Vector3  & n0 = (_pos1 ^ _pos2).Normalize();
					Vector3  & n1 = (_pos1 ^ _pos3).Normalize();;
					// rest angle
					float d = n0 * n1;
					if (d > 1) d = 1;	// prevent numerical drift
					if (d < -1) d = -1;
					float rest_angle = acosf(d);
					// define the bending constraint
					Constraint bc(BENDING, rest_angle,bending_stiffness);
					bc.ref_parts.push_back(&_particles[a]);
					bc.ref_parts.push_back(&_particles[b]);
					bc.ref_parts.push_back(&_particles[c]);
					bc.ref_parts.push_back(&_particles[m]);
					AddConstraint(bc);
				}
			}
		}
	}
	triangles->EndIteration(iter2);
	bend_constriant=_constraints.size() -(stretch_constriant + bone_constriant);
	//cout << "\n Number of Constraints for bend: " << bend_constriant << endl;

}
///////////////////////////// Performance Profiling \\\\\\\\\\\\\\\\\\\\\\\\\  
// Define constraint 
// tetrahedral constraint to maintian the tetrahedral volume */
void ParticleSystem ::DefineTetConstraints()
{

	for (int i = 0; i < _tetras.size() / 4; i++)
	{
		Constraint tet(TETRA_VOLUME, -1.0, volume_stiffness);

		for (int j = 0; j < 4; j++)
		{
			Particle & p = _particles[_tetras[i*4+j]];
			tet.ref_parts.push_back(& p);
		}

		tet.SetRestVolume();
		AddConstraint(tet);
		tet_constriant= _constraints.size()-(bend_constriant + bone_constriant + stretch_constriant);
		//printf("volume: %.5f\n", tet.rest_value);
	}
	//cout << "\n Number of Constraints for Tetras: " << tet_constriant << endl;
}
void ParticleSystem ::DefineVolumeConstraints()
{
	//cout << " Define the volume ========> " << _tetfaces.size() << endl;
	Constraint c(VOLUME, -1.0, volume_stiffness);
	for (int i = 0; i < _particles.size(); i++)
		c.ref_parts.push_back(GetParticleAt(i));

	for (int i = 0; i < _tetfaces.size()/3; i++)
		c.ref_faces.push_back( &(_tetfaces[i * 3]) );

	c.SetRestVolume();
	AddConstraint(c);
	//printf("volume: %.3f\n", c.rest_value);
}
void ParticleSystem :: DrawLinkBone()
{
	glLineWidth(3);
	glColor3f(1,1,0);
	glBegin (GL_LINES);
	for ( int i=0; i<_particles.size(); i++)
	{

		Vector3 par= _particles[i].pos;
		Vector3 bon;
		bon.x= _particles[i].nearestbone.nearpoint.x;
		bon.y= _particles[i].nearestbone.nearpoint.y;
		bon.z= _particles[i].nearestbone.nearpoint.z;
		// cout << " here nadine we draw " ;
		glVertex3f(par.x,par.y,par.z);
		glVertex3f(bon.x,bon.y,bon.z);

	}
	glEnd();
}
/*! \brief: compute the Centroid of teterhderal 
* center of mass
*/
///////////////// NADINE YOU SHOULD TEST THIS ////////////
void ParticleSystem :: ComputeCentroid()
{
	// compute centroid;
	centroid = Vector3();
	bmax = Vector3(-1000000, -1000000, -1000000);
	bmin = Vector3(1000000, 1000000, 1000000);
	for (int i = 0; i < _particles.size(); i++)
	{
		Particle & p = *(GetParticleAt(i));
		const Vector3 & pos = p.pos;

		centroid += pos;
		if (pos.x > bmax.x) bmax.x = pos.x;
		if (pos.y > bmax.y) bmax.y = pos.y;
		if (pos.z > bmax.z) bmax.z = pos.z;
		if (pos.x < bmin.x) bmin.x = pos.x;
		if (pos.y < bmin.y) bmin.y = pos.y;
		if (pos.z < bmin.z) bmin.z = pos.z;
	}

	centroid /= (float) _particles.size();
}
Vec3f ParticleSystem::Maxboundingbox() 
{   
	return boundbox->getMax();

}
Vec3f ParticleSystem::Minboundingbox() 
{   
	return boundbox->getMin();
}
void ParticleSystem::Draw() 
{   

	// glEnable(GL_LIGHTING);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable( GL_POINT_SMOOTH );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glPointSize( 1.0 );
	glColor3f(0.1,0.2,0.8); // green
	glPushMatrix();
	glBegin (GL_POINTS);
	for(int i =0;i<_particles.size();i++ ) 
	{
		Vector3 a = _particles[i].pos;
		glVertex3f(a.x,a.y,a.z);
	}
	glEnd();
	/*glColor3f(0.5,0.6,0.6); 
	//glColor4f(0.5,0.6,0.6,0.4); // green
	Iterator<Triangle*> *iter = triangles->StartIteration();
	glBegin (GL_TRIANGLES);
	while (Triangle *t = iter->GetNext())
	{
	Vec3f a = (*t)[0]->get();
	Vec3f b = (*t)[1]->get();
	Vec3f c = (*t)[2]->get();
	glVertex3f(a.x(),a.y(),a.z());
	glVertex3f(b.x(),b.y(),b.z());
	glVertex3f(c.x(),c.y(),c.z());
	}
	triangles->EndIteration(iter);
	glEnd();*/
	//glLoadIdentity(); // reset all transformations
	//glPopMatrix();
	// draw all the interior, non-crease edges
	//glDisable(GL_LIGHTING);
	// draw all the interior, non-crease edges
	glLineWidth(1.5);
	glColor3f(0.2,0.2,0.2);
	glBegin (GL_LINES);
	Iterator<ParticleEdge*> *iter1 = edges->StartIteration();
	while (ParticleEdge *e = iter1->GetNext())
	{
		Vector3 a =  _particles[(*e)[0]->getIndex()].pos;
		Vector3 b = _particles[(*e)[1]->getIndex()].pos;
		glVertex3f(a.x,a.y,a.z);
		glVertex3f(b.x,b.y,b.z);
	}
	edges->EndIteration(iter1);
	glEnd();
}
void ParticleSystem :: CollisionPlane(Vector3 p, Vector3 n)	// collision with a plane passing for p and with normal n
{
	vector<Particle>::iterator pIt;
	for(pIt = _particles.begin(); pIt != _particles.end(); pIt++) 
		if(!pIt->isFixed)
		{
			Vector3& pos = pIt->pos - p;
			float dot = pos * n;
			if (dot < 0)
			{
				//Vector3 t = pIt->oldPos;
				pIt->oldPos = pIt->pos;
				pIt->pos += n * -dot;
				//pIt->pos = t;
			}
		}
}
double ParticleSystem::CalculateTriArea( Vec3f v1,Vec3f v2,Vec3f v3)
{
	// recive the vertices postion and cacluate the area
	Vec3f cross(0.0,0.0,0.0);
	cross.Cross3(cross,(v3-v1),(v3-v2)); 
	double area = 0.5 * (sqrtf( cross.x()*cross.x() + cross.y()*cross.y() + cross.z()*cross.z()));
	return area;
}
void ParticleSystem::CalculateFaceNormals()
{
	for ( int nTri = 0; nTri < nb_tri; nTri++ )  // for loop the number of triangles 
	{         
		int nV = nTri*3;
		Vec3f P0(_particles[ _faces[ nV ] ].pos.x,_particles[ _faces[ nV ] ].pos.y,_particles[ _faces[ nV ] ].pos.z);
		Vec3f P1(_particles[ _faces[ nV+1 ] ].pos.x,_particles[ _faces[ nV+1 ] ].pos.y,_particles[ _faces[ nV+1 ] ].pos.z);
		Vec3f P2(_particles[ _faces[ nV+2 ] ].pos.x,_particles[ _faces[ nV+2 ] ].pos.y,_particles[ _faces[ nV+2 ] ].pos.z);
		Vec3f cross(0.0,0.0,0.0);
		cross.Cross3(cross,P1-P0,P2-P0); 
		TriNorms[ nTri ] = cross;
		double area =CalculateTriArea(P0,P1,P2);
		// Heron formulas
		double MangV=(P1-P0).Length()*(P2-P0).Length();
		//double divide= ((P1-P0).Normalize().Dot3((P2-P0).Normalize()))/MangV;
		//divide= acos(divide); // find theta as weigth to compute vertex normal
		// WeightAngle[nTri]=divide;
		TriNorms[ nTri ].Normalize(); // normalize base of Magnitude
		triangle.push_back(std::make_pair(nTri,_particles[ _faces[nV] ].pos));
		triangle.push_back(std::make_pair(nTri,_particles[ _faces[nV+1] ].pos));
		triangle.push_back(std::make_pair(nTri,_particles[ _faces[nV+2] ].pos));
	}
	CalculateVerNormals();
}
void ParticleSystem::CalculateVerNormals()
{
	int  NumAdjFace=0;
	GLfloat angle=90;
	GLfloat dot, cos_angle;
	// calculate the cosine of the angle (in degrees) */
	/* calculate the average normal for each vertex */
	// but we should find first all the adjace faces
	for(int i = 0; i < _particles.size(); i++)
	{
		Vec3f node;
		node.Set(_particles[i].pos.x, _particles[i].pos.y, _particles[i].pos.z);  // specific node or vertices to search
		// find frist adj tringle
		////////////////////// Weighted Vertex Normals ///////////////////////////
		for(int j =1; j < triangle.size(); j++)
		{
			if ( triangle[j].second.x==node.x())
				if(triangle[j].second.y==node.y())
					if(triangle[j].second.z==node.z())
						VerNorms[i].Set(VerNorms[i].x()+TriNorms[triangle[j].first].x(),
						VerNorms[i].y()+TriNorms[triangle[j].first].y(),
						VerNorms[i].z()+TriNorms[triangle[j].first].z());

		}
		VerNorms[i].Normalize(); // or Normalize them up
	}
}
void ParticleSystem::ComputeNormals()
{
	// ------------  compute normals for correct lighting
	// compute the normal for each face
	vector<float> WeightAngle;
	Vec3 ee1, ee2;
	for (int i = 0; i < nb_tri; i++)
	{
		int nV = i*3;
		Particle & p0 = *(GetParticleAt(_faces[nV]));
		Particle & p1 = *(GetParticleAt(_faces[nV + 1]));
		Particle & p2 = *(GetParticleAt(_faces[nV + 2]));

		Vector3 normal = (p1.pos - p0.pos) ^ (p2.pos - p0.pos);
		// Heron formulas
		Vector3 e1= (p0.pos - p1.pos);
		Vector3 e2 = (p2.pos - p1.pos);
		ee1.X()=e1.x;
		ee1.Y()=e1.y;
		ee1.Z()=e1.z;
		ee2.X()=e2.x;
		ee2.Y()=e2.y;
		ee2.Z()=e2.z;
		float wedgeAngle = angle(ee1,ee2);
		WeightAngle.push_back(wedgeAngle);
		normal.Normalize();
		_normals[i] = normal;

	}
	// assign the normal to the vertices
	for (int i = 0; i < _particles.size(); i++)
	{
		Particle & p = *(GetParticleAt(i));
		p.normal = Vector3(0, 0, 0);

		if (p.faces.size() > 0)
		{
			set<int>::iterator it = p.faces.begin();
			for (; it != p.faces.end(); it++)
			{
				p.normal += _normals[*it]*WeightAngle[*it];
			}
			//p.normal /= (int) p.faces.size();
			//				p.normal = Vector3(p.pos.x, 0, p.pos.z);
			p.normal.Normalize();
			VerNorms[i].Set(p.normal.x,p.normal.y,p.normal.z);
			RestVerNorms[i].Set(p.normal.x,p.normal.y,p.normal.z);
		}
	}
	// ------------  end compute normals 
}
void ParticleSystem::setBindPose()
{
	_particles=_bindparticles;
	_faces= _bindfaces;
	VcgOrient(*this);
	ComputeNormals();
}
void ParticleSystem ::computeNormals()
{

	/*	// a map from positions to normals
	std::map< Vector3, Vector3 > p2n;
	Vec3 ee1, ee2;
	// pass 1: reset tang and bitang
	for ( int vi=0; vi<_particles.size(); vi++){
	//	p2n[ _particles[vi].pos ] =Vector3(0,0,0);
	}

	// pass 2: cycle over faces, cumulate norms
	for (int i = 0; i < nb_tri; i++)
	{
	int nV = i*3;
	Particle & p0 = *(GetParticleAt(_faces[nV]));
	Particle & p1 = *(GetParticleAt(_faces[nV + 1]));
	Particle & p2 = *(GetParticleAt(_faces[nV + 2]));
	Vector3 normal = (p1.pos - p0.pos) ^ (p2.pos - p0.pos);
	//normal.Normalize();
	Vector3 faceNorm= normal; // this includes area weighting

	for (int z=0; z<3; z++) 
	{
	Particle & p0 = *(GetParticleAt(_faces[(z+1)%3]));
	Particle & p1 = *(GetParticleAt(_faces[z]));
	Particle & p2 = *(GetParticleAt(_faces[(z+2)%3]));
	Vector3 e1= (p0.pos - p1.pos);
	Vector3 e2 = (p2.pos - p1.pos);
	ee1.X()=e1.x;
	ee1.Y()=e1.y;
	ee1.Z()=e1.z;
	ee2.X()=e2.x;
	ee2.Y()=e2.y;
	ee2.Z()=e2.z;
	float wedgeAngle = angle(ee1,ee2);
	p2n[ _particles[ _faces[z] ].pos ] += faceNorm * wedgeAngle;

	}

	}

	// pass 3: normalize
	for (int vi=0; vi<_particles.size(); vi++)
	{
	VerNorms[vi].Set( p2n[ _particles[vi].pos ].Normalize().x,
	p2n[ _particles[vi].pos ].Normalize().y,
	p2n[ _particles[vi].pos ].Normalize().z);

	}*/
}