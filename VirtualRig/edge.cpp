#include "edge.h"
// constructor by sending the vertices and tringles 
ParticleEdge::ParticleEdge(EVertex *v, Triangle *t) {
	vertex = v;
	triangle = t;
	next = NULL;
	opposite = NULL;
	crease = 0;
}
// just opposite could be NULL
ParticleEdge::~ParticleEdge() { 
	if (opposite != NULL)
		opposite->opposite = NULL;
}
// extract vertices belong to specific edge 
// and edge have two vertices 
void ParticleEdge::extract_func(ParticleEdge *e, int &a, int &b, int &c) {
	a = e->getVertex()->getIndex(); 
	b = e->getNext()->getNext()->getVertex()->getIndex();
	c = 0;
}
// edges sending from particle system
/*void ParticleEdge::setEdgefromParticleSystem(int a, int b)
{
	PEdge.first=a;
	PEdge.second=b;
}*/