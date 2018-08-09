#include "bvh2skel.h"
#include "munkres.h"
/*! 
* \brief:
* Param :  BVH object 
*          Skeleton object
*          hungarian algorithm used to find best fitting between the BHV bones and bones
*          exist in skeleton
*          Works based :
*          MIN THE DISTANCE BETWEEN THE SKEL AND BVH FILE
*          AND THIS IS DONE JUST ONE TIME NOT FOR EACH FRAME
*/
void Bvh2skel::MapBvhToSkel(BVH* bvh, CSkeleton * skel)
{
	// for each vertex in the skeleton, compute all the distances
	// with each vertex(joint) in the bvh
	// store all the distances in a matrix
	// is the number of joints in bvh is the same number of joints in skelt ??!!!!
	// note this is an assignment problem, which solvable using a combinatorial optimization algorithm, in this case the hungarian algorithm
	// http://en.wikipedia.org/wiki/Hungarian_algorithm

	vector<float> distances;
	int h = skel->bone.size();
	int w = bvh->GetNumJoint();
     cout << "\n The number of Joint in bvh" <<  w << endl;
	cout << "\n The number of Joint in skel" << h << endl;
	Matrix<double> matrix(h, w);

	for (int i = 0; i < h; i++)
	{
		const Point3f & pa = skel->vert[skel->bone[i].node];

		for (int j = 0; j < w; j++)
		{
			const BVH::Joint * joint = bvh->GetJoint(j);
			const Point3f & pb = joint->wc;

			float d = (pa - pb).Norm();

			//			distances.push_back(d);
			matrix(i, j) = d;
		}
	}

	// Apply Munkres algorithm to matrix.
	Munkres m;
	m.solve(matrix);
	for ( int row = 0 ; row < h ; row++ ) 
	{
		int rowcount = 0;
		for ( int col = 0 ; col < w ; col++  ) 
		{
			if ( matrix(row,col) == 0 )
				rowcount++;
		}
#ifdef DEBUG
		if ( rowcount != 1 )
			std::cerr << "Row " << row << " has " << rowcount << " columns that have been matched." << std::endl;
#endif
	}

	for ( int col = 0 ; col < w ; col++ ) 
	{
		int colcount = 0;
		for ( int row = 0 ; row < h ; row++ ) 
		{
			if ( matrix(row,col) == 0 )
				colcount++;
		}
#ifdef DEBUG
		if ( colcount != 1 )
			std::cerr << "Column " << col << " has " << colcount << " rows that have been matched." << std::endl;
#endif
	}
#if 0

	// Display solved matrix.
	for ( int row = 0 ; row < h ; row++ ) 
	{
		for ( int col = 0 ; col < w ; col++ ) 
		{
			std::cout.width(2);
			std::cout << matrix(row,col) << ",";
		}
		std::cout << std::endl;
	}

	std::cout << std::endl;

#endif

	bvh_to_skel.clear();
	skel_to_bvh.clear();
	skel_to_bvh.assign(skel->bone.size(), -1);
	bvh_to_skel.assign(bvh->GetNumJoint(), -1);
	for (int i = 0; i < h; i++)	// for each skeleton bone
		for (int j = 0; j < w; j++)	// for each bvh joint
			if (matrix(i, j) == 0)
			{
				skel_to_bvh[i] = j;
				bvh_to_skel[j] = i;
			}


#if 0
			// now find the mapping which minimalize the sum of the distances

			// according to the hungarian algorithm, 
			//	for each row
			//		1) find the minimal value in that row
			//		2) subtract m to all the elements of the row
			//		3) map the i-th column with the j-th row if distances (i, j) == 0 
			//			warning => this latter is not a univoque operation

			for (int i = 0; i < h; i++)	// for each skeleton bone
			{
				// 1)
				float min_d = numeric_limits<int>::max();
				for (int j = 0; j < w; j++)
				{
					float value = distances[i * w + j];
					if (value < min_d)
						min_d = value;
				}

				// 2)
				for (int j = 0; j < w; j++)
					distances[i * w + j] -= min_d;

				// 3)
				for (int j = 0; j < w; j++)	// for each bvh joint
					if (distances[i * w + j] < 0.0001)
					{
						skel_to_bvh.push_back(j);
						// warning!! this is wrong sometimes!! see step 2 in http://en.wikipedia.org/wiki/Hungarian_algorithm
					}
			}
#endif
}
