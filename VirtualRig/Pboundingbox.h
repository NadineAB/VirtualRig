#ifndef _PBOUNDING_BOX_H_
#define _PBOUNDING_BOX_H_

#include <assert.h>
#include "vectors.h"
#include "utils.h"
// ====================================================================
// ====================================================================
class PBoundingBox {

public:

  // CONSTRUCTOR & DESTRUCTOR
  PBoundingBox(Vec3f _min, Vec3f _max) {
    Set(_min,_max); }
  ~PBoundingBox() {}

  // ACCESSORS
  void Get(Vec3f &_min, Vec3f &_max) const {
    _min = min;
    _max = max; }
  Vec3f getMin() const { return min; }
  Vec3f getMax() const { return max; }

  void getCenter(Vec3f &c) const {  // return the center of the bounding box
    c = max; 
    c -= min;
    c *= 0.5f;
    c += min;
  }

  double maxDim() const {
    double x = max.x() - min.x();
    double y = max.y() - min.y();
    double z = max.z() - min.z();
    return max3(x,y,z);
  }

  // MODIFIERS
  // set the bounding box
  void Set(PBoundingBox *bb) { // by bounding box
    assert (bb != NULL);
    min = bb->min;
    max = bb->max; }
  void Set(Vec3f _min, Vec3f _max) { // by vector
    assert (min.x() <= max.x() &&
	    min.y() <= max.y() &&
	    min.z() <= max.z());
    min = _min;
    max = _max; }
  void Extend(const Vec3f v) { // extend bounding box by vector 
    min = Vec3f(min2(min.x(),v.x()),
		min2(min.y(),v.y()),
		min2(min.z(),v.z()));
    max = Vec3f(max2(max.x(),v.x()),
		max2(max.y(),v.y()),
		max2(max.z(),v.z())); }
  void Extend(PBoundingBox *bb) { // extend bounding box by value
    assert (bb != NULL);
    Extend(bb->min);
    Extend(bb->max); }

  // DEBUGGING 
  void Print(const char *s="") const {
    printf ("BOUNDING BOX %s: %f %f %f  -> %f %f %f\n", s,
            min.x(),min.y(),min.z(),
            max.x(),max.y(),max.z()); }
  void paint() const;

private:
  PBoundingBox() { assert(0); } // don't use this constructor

  // REPRESENTATION
  Vec3f min;
  Vec3f max;
};

// ====================================================================
// ====================================================================

#endif
