#ifndef MATERIAL_H
#define MATERIAL_H
#pragma warning (disable: 4002)
// include headers that implement a archive in xml format
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/filesystem.hpp>
#include <map>
#include <string.h>
#include <fstream>
#include<vector>
#include <vcg/complex/complex.h>
#include <vcg/complex/algorithms/create/platonic.h>
#include <vcg/complex/algorithms/update/bounding.h>
#include <vcg/complex/algorithms/update/normal.h>
#include <vcg/complex/algorithms/update/topology.h>
#include <vcg/complex/algorithms/update/curvature.h>
#include <vcg/simplex/face/pos.h> 
#include <vcg/space/index/grid_static_ptr.h>
#include <vcg/space/index/grid_static_obj.h>
#include <vcg/space/segment3.h>
#include <vcg/space/point3.h>
#include <vcg/space/intersection3.h>
#include <vcg/space/ray3.h>
#include <vcg/math/matrix33.h>
using namespace std;
using namespace vcg;
/*! 
* \brief:
* class interface to load material attached to mesh
* coded by Marco Fratarcangeli
*/
class CMaterial
{
private:
	friend class boost::serialization::access;
	template<class Archive> void serialize(Archive & ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(illum);
		ar & BOOST_SERIALIZATION_NVP(Ns);
		ar & BOOST_SERIALIZATION_NVP(textures);
	}

public:

	CMaterial();
	CMaterial(int _i, float _n) : illum(_i), Ns(_n){}

	unsigned int index;		//index of material
	string materialName;

	Point3f Ka;				//ambient
	Point3f Kd;				//diffuse
	Point3f Ks;				//specular

	int illum;//specular illumination
	float Ns;

	map<string, string> textures; //role, filename texture
};
#endif // __MATERIAL__