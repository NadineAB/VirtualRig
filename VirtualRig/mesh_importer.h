#ifndef mesh_importer__
#define mesh_importer__
/*! 
* \brief:
* Base class used to import tetrahedral mesh in order to simulate the volumatic
* mesh [ which helps to preserve volume ]
* coded by Marco Fratarcangeli
*/
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
//#include "vcg_orient.h"
#include "ParticleSystem.h"
class MeshImporter
{

public:
	void Read(string & filename, ParticleSystem & ps);
	void ImportVertices(std::ifstream & i_file, ParticleSystem & ps);
	void ImportFaces(std::ifstream & i_file, ParticleSystem & ps);
	void ImportTetras(std::ifstream & i_file, ParticleSystem & ps);
};

#endif

