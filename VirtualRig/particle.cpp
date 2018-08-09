#include "particle.h"
#include "Vector3.h"

using namespace std;
/*! \brief:  Class interface for particle
(which deal with vertices as particle where the prevouis postion, current postion 
old postion are matter
*/

Particle:: Particle()
{
	Init();
}
// compute the previous postion of vertices
Particle::Particle(double x, double y, double z)
{
	oldPos = restPos = pos = Vector3(x, y, z);
	errorpos =0;
	errorvol=0;
	// is the reset pos same as previous pose
	Init();
}

Particle::Particle(Vector3 _p)
{
	oldPos = restPos = pos = _p;
	Init();
}
void Particle ::Init()
{
	neighs.clear();
	faces.clear();
	acc = Vector3(0, 0, 0);
	grad = Vector3(0, 0, 0);
	normal = Vector3(0, 0, 0);
	isFixed = false;
}
/*! 
* \brief:
* Param : reviced object of boneweight
* which assgin each particle with specific weight
*/
void Particle::PushPartBoneWeight(CPartWeight & bw)
{
	for (vector<CPartWeight>::iterator it = particle_weight.begin(); it != particle_weight.end(); it++)
	{
		if (it->bone == bw.bone)
			return;
	}

	particle_weight.push_back(bw);
}
/*! 
* \brief:
* Param : reviced object of boneweight
* which assgin each particle with specific smooth weight
*/
void Particle::PushSmoothBoneWeight(CPartWeight & bw)
{
	for (vector<CPartWeight>::iterator it = smoothparticle_weight.begin(); it != smoothparticle_weight.end(); it++)
	{
		if (it->bone == bw.bone)
			return;
	}
	

	smoothparticle_weight.push_back(bw);

}
/*! 
* \brief:
* Param : reviced object of boneweight
* Normalize weight
*/
void Particle::NormalizeParWeights()
{
	//	printf( " NormalizeParWeights ");

	if (particle_weight.empty())
		return;
	float sum = 0;
	for (vector<CPartWeight>::iterator it = particle_weight.begin(); it != particle_weight.end(); it++)
		sum += it->weight;

	for (vector<CPartWeight>::iterator it = particle_weight.begin(); it != particle_weight.end(); it++)
		it->weight = it->weight / sum;

	// check for weights with sum magnitude very low (e.g. they are zero)
	if (sum < 0.1)
		particle_weight.clear();

}