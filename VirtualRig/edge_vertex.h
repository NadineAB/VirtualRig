#ifndef _VERTEX_H
#define _VERTEX_H
//*! 
// \brief:
// Class to define vertices of vector 
// this class used to create half-edge data structure 
//==========================================================
#include <stdio.h>
#include <assert.h>
#include "vectors.h"
class EVertex;
// ==========================================================
class EVertex {

public:

	// ========================
	// CONSTRUCTOR & DESTRUCTOR
	EVertex(int i, const Vec3f &pos) : //CONSTRUCTOR
	 position(pos) { index = i; } // index vertices and postion
	 virtual ~EVertex() { } // DESTRUCTOR

	 // =========
	 // ACCESSORS
	 int getIndex() const { return index; } // get the vertices index
	 double x() const { return position.x(); } // get vertex postion x
	 double y() const { return position.y(); } // get vertex postion y
	 double z() const { return position.z(); } // get vertex postion z
	 const Vec3f& get() const { return position; } // return vector of postion 

	 // =========
	 // MODIFIERS
	 void set(Vec3f v) { position = v; } // to insert new postion as vector 
	 void set(double x, double y, double z) { position.Set(x,y,z); } // insert new postions

private:
	// some opertation we migth use to compare between two vertices, 
	// assign vertices to new one
	EVertex() {}
	EVertex assign(const EVertex& v1) 
	{ 
		return v1; // assgin the value in vertex1 in to the calling instance
	}
	bool equal(const EVertex& v1,const EVertex& v2)
	{ 
		if( v1.x()==v2.x() && v1.y()==v2.y() && v1.z()==v2.z())
			return true;  // if they are the same vertices equal
		else 
			return false;
	}
	EVertex(const EVertex&) { assert(0); }
	// ==============
	// REPRESENTATION
	Vec3f position;
	// this is the index from the original .obj file.
	// technically not part of the half-edge data structure
	int index;  
	// NOTE: the vertices don't know anything about adjacency.  In some
	// versions of this data structure they have a pointer to one of
	// their incoming edges.  However, this data is very complicated to
	// maintain during mesh manipulation.
};
// ==========================================================

#endif

