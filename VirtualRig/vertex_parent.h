#ifndef VERTEX_PARENT_H
#define VERTEX_PARENT_H
#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include "utils.h"
#include "edge_vertex.h"
// ===================================================================
/*!\brief:VertexParent
// useful when you need to track the hierarchy of vertices.  this data
// structure can be placed in a Bag, and then accessed with the
// extract function to find the child vertex (if it exists) when given
// two parents.*/
// coded by Leonardo Fischer
// ===================================================================
// VertexParent class contain three different vertices 
class VertexParent { 
public:
	// ========================
	// CONSTRUCTORS & DESTRUCTOR
	//CONSTRUCTORS
	VertexParent(EVertex *_p1, EVertex *_p2, EVertex *_v) {
		// terminal from program if return flase conditions
		assert (_p1 != NULL && _p2 != NULL && _v != NULL); // if three vertices are equal null
		assert (_p1 != _p2 && _p1 != _v && _p2 != _v); // if p1 not equal 
		// and p1 not equal v and p2 not equal v
		// to ensure nothing equal to null
		// not any of vertices equal the other not the same
		p1 = _p1;
		p2 = _p2;
		v = _v;
	}
	~VertexParent(); //DESTRUCTOR

	// to be put in a bag...
	// where bag is The bag class is for unordered data. For efficient (constant time)
	// retrieval and removal of elements
	// so we sent parent vertex  and I need to get the index for 
	// the three vertices, so by sending three integer call by referance
	// and update these values inside this function
	// static in c++ 
	// static not visible outside this class

	static void extract_func(VertexParent *e, int &a, int &b, int &c) {
		a = std::min2(e->p1->getIndex(),e->p2->getIndex()); 
		// in integer a store the min integer 
		b = std::max2(e->p1->getIndex(),e->p2->getIndex());
		c = 0;
	}

	// =========
	// ACCESSORS
	EVertex* get() const { return v; } // get the parent vertex

protected:
	// 
	VertexParent(const VertexParent&) { assert(0); }
	VertexParent operator=(const VertexParent v) { return v; }

	// ==============
	// REPRESENTATION
	EVertex *p1;
	EVertex *p2;
	EVertex *v;
};

// ===================================================================

#endif