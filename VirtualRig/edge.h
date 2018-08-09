#ifndef PARTICLEEDGE_H
#define PARTICLEEDGE_H
#include <limits.h>
#include <stdio.h>
#include <assert.h>
#include <unordered_set>
#include <boost/functional/hash.hpp>
#include "edge_vertex.h"
class EVertex; // we need to Vertex need 
class Triangle; // we need to Triangle need 
// ===================================================================
////*! 
// \brief: half-edge data structure
// \ coded by Leonardo Fischer 
// \ updated
//====================================================================
class PEDGE
{
public:
std::pair<int,int>  pedge; // store the result of searching
PEDGE (){};
void setPEDGE (int x, int y )
{
	pedge.first=x;
	pedge.second=y;
}
bool operator==( PEDGE edge) const 
{
	return ((int)pedge.first == (int)edge.pedge.first) && ((int)pedge.second == (int)edge.pedge.second);
}
bool operator<( PEDGE  edge) const
{
	return (pedge.first < edge.pedge.first  || (pedge.first == edge.pedge.first && pedge.second < edge.pedge.second));
	//return (int)pedge.first > (int)edge.pedge.first && (int)pedge.second > (int)edge.pedge.second ;

}
};
class ParticleEdge { 
public:
	// ========================
	// CONSTRUCTORS & DESTRUCTOR
	ParticleEdge(EVertex *v, Triangle *t); // CONSTRUCTORS
	~ParticleEdge(); //DESTRUCTOR
	ParticleEdge(); //CONSTRUCTOR
	ParticleEdge(int e1, int e2){setEdgefromParticleSystem(e1,e2);} // with two edges
	// here's the hash function to use for edges so they
	// can be efficiently accessed within the Bag data structure
	
	static void extract_func(ParticleEdge *e, int &a, int &b, int &c); 
	// extract function that take as input edge and 3 integer which 
	// they are vertices index 
	// =========
	// ACCESSORS
	EVertex* getVertex() const { assert (vertex != NULL);
	return vertex; } 
	// get vertex for specific edge but it cant equal null
	ParticleEdge* getNext() const 
	{ assert (next != NULL);
	return next; }
	// get the next edge to adjacency edge and cant equal null
	Triangle* getTriangle() const { assert (triangle != NULL); return triangle; }
	// get the triangle that this belong to 
	ParticleEdge* getOpposite() const {
		// warning!  the opposite edge might be NULL! 
		return opposite; }
	float getCrease() const { return crease; }
	// Crease its float value { I dont knew indicate to what }
	EVertex* operator[](int i) const { // interst into 0 or the vertex that
		// belong to exact edge if 1 return the vertex that belong to the
		// next edge
		if (i==0) return getVertex();
		if (i==1) return getNext()->getNext()->getVertex();
		assert(0);
	}
	// =========
	// MODIFIERS
	// set values in opposite edge [ this is edge ]
	// so we need to check that its not equal to Null 
	// then assgin the value complexity O(4)
	void setOpposite(ParticleEdge *e) {
	//	assert (opposite == NULL); // exit if NULL
		assert (e != NULL); // make sure that edge not equal NULL
		assert (e->opposite == NULL); // exit if 
		opposite = e; 
		e->opposite = this; 
	}
	// clear the value assgin to opposite edge 
	// by puting equal null
	void clearOpposite() { 
		if (opposite == NULL) return; 
		assert (opposite->opposite == this); 
		opposite->opposite = NULL;
		opposite = NULL; 
	}
	// set next edge O(3)
	// insure value not equal to NULL
	// becareful nadine I made some change here 
	// comment tow assert
	void setNext(ParticleEdge *e) {
		assert (next == NULL);
		assert (e != NULL);
		assert (triangle == e->triangle);
		next = e;
	}
	void setCrease(float c) { crease = c; } // set crease values
	void setEdgefromParticleSystem(int a, int b); // set Edges from particel system
	PEDGE PEdge;  // Edge sent from Paticlesystem  
	//ParticleEdge& operator=(const ParticleEdge&) { assert(0) return 0; } //  asgin 
	

private:
	// ==============
	// REPRESENTATION
	// in the half edge data adjacency data structure, the edge stores everything!
	EVertex *vertex; // vertex pointer
	Triangle *triangle; // trinagle 
	ParticleEdge *opposite; // opposite edge 
	ParticleEdge *next; // next edge
	float crease;
};

// ===================================================================

#endif
