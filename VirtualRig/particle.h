#ifndef _PARTICLE__
#define _PARTICLE__


#include <vector>
#include <vcg/math/base.h>
#include <set>
/*! \brief:  
// Class interface for particle
// (which deal with vertices as particle where the prevouis postion, current postion 
// old postion are matter
// as assgin weigth and bone for each particle element
// coded by Marco Fratarcangeli
*/
using namespace std;
#include "Vector3.h"

class CBone;
class Particle
{
	/*GAUSS_SEIDEL_RELAX
	JACOBI
	delta_p*/
public:
	class CPartWeight
	{
	public: 
		CBone * bone;	     // the bone object
		float weight;	     // the weight of the i-th bone
		float tp;	     // tp
	     float distance;	// the weight of the i-th bone
		Vector3 mdelta;     // the distance between the particle and bone in reset bone
		Vector3 lcoord;	// coordinates of the vertices in the local frame of the bone
	     Vector3 nearpoint;	// coordinates of the nearest point of the sagment
		// Vector3 wcoord;    // coordinate in world
	};
	CPartWeight nearestbone;  // Vector of class  vertex , weight , local coord of the bone
	vector<CPartWeight> particle_weight;  // Vector of class  vertex , weight , local coord of the bone
	vector<CPartWeight> smoothparticle_weight;  // Vector of class  vertex , weight , local coord of the bone
	void PushPartBoneWeight(CPartWeight & bw); // assgin weight for each particle
	void PushSmoothBoneWeight(CPartWeight & bw); // assgin smooth weight for each particl
	void NormalizeParWeights(); // normalize particle weight according to all particles
	// error in postion 
	double errorpos;	
	// error in volume 
	double errorvol;	
	// current postion 
	Vector3 pos;		
	// reset postion 
	Vector3 restPos;
	// prevouis postion
	Vector3 oldPos;		
	// acceleration
	Vector3 acc;		
	// vertices index
	int index;
	Vector3 delta_p;
	// rigid
	bool isFixed;	
	bool isExternal;	
	// gradient which consider here the slope of the acceleration  
	Vector3 grad;
	// neighborhood 
	set<int> neighs;
	float Pstretch_stiffness;
	float Pstet_stiffness;
	float Pbone_stiffness;
	// mesh faces
	set<int> faces;
	// set of teterhderal
	set<int>	tetras; 
	// smooth normal (vertices)
	Vector3 normal;
	// empty constructor 
	Particle();
	// construct take vector3 as parameter  
	Particle(Vector3 _p);
	// construct take 3 float numbers as parameter  
	Particle(double x, double y, double z);
	// inilization all particle element
	void Init();
	// operator <
	bool operator<(const Particle& p) const {
			return Particle() < p;
		}
};

# endif // _PARTICLE__