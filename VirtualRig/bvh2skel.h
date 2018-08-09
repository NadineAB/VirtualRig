#ifndef  _BVH2SKEL_H_
#define  _BVH2SKEL_H_
#include "skeleton.h"
#include "bvh.h"
#include <map>
#include <vector>

using namespace std;
using namespace vcg;
/*! 
* \brief:
* Base class which map from BVH and Skel
* coded by Marco Fratarcangeli
*/
class        Bvh2skel
{
public:

	vector<int> bvh_to_skel;
	vector<int> skel_to_bvh;
	// method that map BVH to Skel
	void MapBvhToSkel(BVH* bvh, CSkeleton * skel);
};
#endif