#ifndef __CONSTRAINT__
#define __CONSTRAINT__
#include <vector>
#include <assert.h>
#include "particle.h"
#include "bag.h"
#include "Triangle.h"
using namespace std;
/*!
* \brief:  Class interface for constriant the particles system  
* \contain
* stretch , blending , area , volume ( which required to teterhdera)  
* coded by Marco Fratarcangeli
*/
extern vector<pair<Vector3, Vector3>> debug_lines;
enum SolverType { GAUSS_SEIDEL_RELAX,JACOBI};
enum ConstraintType { STRETCH, BENDING, BONESTRETCH , AREA, VOLUME, TETRA_VOLUME, CONSTR_NONE };
// represents a spatial constraint
// see "Muller, Heidelberger, Hennix, Ratcliff - Position Based Dynamics"
// available for free here http://www.matthiasmueller.info/publications/publications.htm
class CSkeleton;
class Constraint
{
public:
	ConstraintType type;
	// reference to the particles affected by this constraint
	vector<Particle *> ref_parts;	
	vector<int*> ref_faces;		// reference to the faces affected by this constraint
	vector<Particle::CPartWeight *> ref_bones;	
	// reference to the faces affected by this constraint
	//Bag<Triangle*> *ref_faces;		
	CSkeleton * skel;
	float stiffness;				// stiffness value (see paper), 0 <= stiffness <= 1
	double rest_value;				// value which satisfies the constraint

	int phase;	// in which phase the constraint is solved
	// empty constructor
	Constraint();
	~Constraint(){
		/*delete ref_faces;
		ref_faces = NULL;*/} 
	// constructor with constraint type and default values  
	Constraint(ConstraintType _type, float _rest_value,float _stiffness);
	// assign value for the rest volume of the skin
	void SetRestVolume();
	// apply any type of constraint on the mesh 
	void Solve(SolverType solver_type);
	// this function to solve stretch bone 
	void SolveStretchBone(SolverType & solver_type);
	void SolveStretch(SolverType & solver_type); 
	// this function to solve bending 
	void SolveBending(SolverType & solver_type);
	// this function to tringle area
	void SolveTriArea(SolverType & solver_type);
	// this function to solve volume of tetradera 
	void SolveVolume(SolverType & solver_type);
	void SolveVolumeTetra(SolverType & solver_type);
protected:
	// compute volume in order to preserve volume 
	float ComputeVolume();
	// compute volume using tetraderah
	float ComputeVolumeTetra();
};

#endif // __CONSTRAINT__