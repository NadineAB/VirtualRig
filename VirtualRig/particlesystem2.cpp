#define NUM_ITERATIONS 4	// max number of iteration to satisfy the constraints	
#define TIME_STEP 0.01f
#include "skeleton.h"
#include "particle.h"
#include "constraint.h"
#include "particlesystem.h"
#include <vector>
using namespace std;
#define INITIAL_VERTEX 10000000
#define INITIAL_EDGE 10000000
#define INITIAL_TRIANGLE 10000000
/////// define profile and profile node to estimate code proformance 
#include "profile.h"
#define USE_PROFILE		// undef to disable profiling
////////////////////////////////////////////////////////////////////
//default value of the particle system
ParticleSystem::ParticleSystem()
{
	_particles.reserve(100000);
	_constraints.reserve(100000);
	_faces.reserve(100000);
	_tetras.reserve(100000);
	m_fTimeStep = TIME_STEP;
	m_iNbIterations = NUM_ITERATIONS;
	stretch_stiffness = 1.f;
	bending_stiffness = 1.f;
	volume_stiffness = 1.f;
	edges = new Bag<ParticleEdge*>(INITIAL_EDGE,ParticleEdge::extract_func);
	vertices = new Array<EVertex*>(INITIAL_VERTEX);
	triangles = new Bag<Triangle*>(INITIAL_TRIANGLE,Triangle::extract_func);
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
}
// set timestep value
void ParticleSystem :: SetTimeStep(float ts)		
{
	if (ts > 0)
		m_fTimeStep = ts;
}
float ParticleSystem ::GetTimeStep() {return m_fTimeStep;}   // return the timestep
void ParticleSystem::SetNbIterations(int ni)	
{
	if (m_iNbIterations > 0)
		m_iNbIterations = ni;
}
int ParticleSystem::GetNbIterations() {return m_iNbIterations;}
// return particle in specific index
Particle* ParticleSystem ::GetParticleAt(int i)
{
	assert(i < (int)_particles.size());
	return &_particles[i];
}
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
	_particles.push_back(_p);
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
}
// add Faces
//c.1: ParticleSystem:: AddFace complexity O(10) for each face
/*****************************************************************************/
void ParticleSystem ::AddFace(int id[3])
{
	PROFILE_SAMPLE("AddFace");
	int id_face = _faces.size() / 3;
	for (int j = 0; j < 3; j++)
	{
		_faces.push_back(id[j]);
		Particle & p = *(GetParticleAt(id[j]));
		p.faces.insert(id_face);
	}
}
///////////////////////////// Performance Profiling \\\\\\\\\\\\\\\\\\\\\\\\\  
//c.2: ParticleSystem:: add edges
/*****************************************************************************/
void ParticleSystem ::AddPEdges(int id[3])
{
	PROFILE_SAMPLE("AddPEdges");
	// for each face index loop of complexity O(42)
	for (int i = 0; i < 3; i++)
	{
		int i0 = id[i];
		int i1 = id[(i + 1) % 3];
		if (i0 > i1)
			swap(i0, i1);
		EVertex *va = GetVertex(i0); // complexity O(3)
		EVertex *vb = GetVertex(i1); // complexity O(3)
		ParticleEdge *ab = GetEdge(va,vb); // GetEdge complexity O(2) 
		ParticleEdge e(i0, i1);
		_edges.push_back(e);
	}
	// append memory space for the normal of this face
	//_normals.push_back(Vector3());
}
void ParticleSystem::AddTetra(int id[4])
{
}
// set gravity O(1)
void ParticleSystem::SetGravity(Vector3 g)
{    PROFILE_SAMPLE("SetGravity");
m_vGravity = g;
}
void ParticleSystem::AccumulateForces()
{
	PROFILE_SAMPLE("AccumulateForces");
	for(int i=6; i<_particles.size(); i++) 
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

void ParticleSystem ::DefineSystem(
	int nb_vertices, 
	int nb_faces, 
	int nb_edges, 
	vector<Particle> _p,
	vector<int> _f ,
	vector<ParticleEdge> _e)
{
	PROFILE_SAMPLE("DefineSystem");
	Reset();
	//cout<< "\n Define Particle System" ;
	_particles =_p;
	_faces=_f;
	_edges=_e;
	//cout<< "\n Define Constriants " ;
	DefineConstraints();
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
			x += x - oldx + a * m_fTimeStep * m_fTimeStep ;
			oldx = temp;
		}
}		
void ParticleSystem::SatisfyConstraints() 
{
	//cout << _constraints.size();
	PROFILE_SAMPLE("SatisfyConstraints");
	for(int j = 0; j < NUM_ITERATIONS; j++) 
	{	
		vector<Constraint>::iterator cIt;
		for(cIt = _constraints.begin(); cIt != _constraints.end(); cIt++) 
		{
			//cout <<cIt->rest_value; 
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
	EVertex *v = new EVertex(index, position);
	vertices->Add(v);
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
	triangles->Add(t); 
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
	PROFILE_SAMPLE("DefineConstraints");
	Iterator<ParticleEdge*> *iter1 = edges->StartIteration();
	while (ParticleEdge *e = iter1->GetNext()) 
	{
		if (solve_types[STRETCH])
		{
			int i0 = (*e)[0]->getIndex();
			int i1 = (*e)[1]->getIndex();
			Particle & p0 = _particles[i0];
			Particle & p1 = _particles[i1];
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
				Constraint c(STRETCH, (p0.pos - p1.pos).Length());
				c.ref_parts.push_back(&p0);
				c.ref_parts.push_back(&p1);
				AddConstraint(c);
			}
		}
	}
	edges->EndIteration(iter1);
	// define bending constraints whereever there are no boundary
	// when e->getOpposite()== NULL this is a border edge, so skip it
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
	// Define bone constraints which bind the skin vertices to the bone 
	
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