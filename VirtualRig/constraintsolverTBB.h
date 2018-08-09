#include "timer.h"
#include "edge.h"
#include "constraint.h"
#include <vector>
#include <set>

#include "tbb/tbb.h"

using namespace tbb;
using namespace std;
#define NUM_ITERATIONS 4// numero massimo di iterazioni per soddisfare i vincoli per rilassamente
// max number of iteration to satisfy the constraints
#define TIME_STEP 0.001f	
class ConstraintSolverTBB
{
	vector<Constraint> * pconstraints;
	vector<int> * pbatch;

public:
	void operator ()(const blocked_range<int> & r) const
	{
		vector<Constraint> & constraints = *pconstraints;
		vector<int> & batch = *pbatch;
		for (size_t i = r.begin(); i != r.end(); ++i)
			constraints[batch[i]].Solve(GAUSS_SEIDEL_RELAX);
	}

	ConstraintSolverTBB(vector<Constraint> & constraints, vector<int> & batch)
	{
		pconstraints = &constraints;
		pbatch = &batch;
	}
};

class VerletTBB
{
	vector<Particle> * pparticles;

public:
	void operator ()(const blocked_range<int> & r) const
	{
		vector<Particle> & particles = *pparticles;
		for (size_t i = r.begin(); i != r.end(); ++i)
		{
			Particle & p = particles[i];
			if(!p.isFixed)
			{
				Vector3& x = p.pos;
				Vector3  temp = x;
				Vector3& oldx = p.oldPos;
				Vector3& a = p.acc;

				x += x - oldx + a * TIME_STEP * TIME_STEP /*- ((x - oldx)) * 0.000003f*/;
				
				oldx = temp;
			}
		}
	}

	VerletTBB(vector<Particle> & particles)
	{
		pparticles = &particles;
	}
};
