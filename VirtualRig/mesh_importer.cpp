#define _CRT_SECURE_NO_WARNINGS
#  pragma warning(disable:4002)
#include "mesh_importer.h"
#include<vcg/complex/complex.h>
#include<vcg/complex/algorithms/create/platonic.h>

//using namespace vcg;
using namespace std;

/*class MyEdge;
class MyFace;
class MyVertex;
struct MyUsedTypes : public UsedTypes<	Use<MyVertex>   ::AsVertexType,
                                        Use<MyEdge>     ::AsEdgeType,
                                        Use<MyFace>     ::AsFaceType>{};

class MyVertex  : public Vertex<MyUsedTypes,  vertex::Coord3f, vertex::Normal3f, vertex::VFAdj, vertex::Qualityf, vertex::Color4b, vertex::BitFlags  >
{
public:
	int index;
};
class MyFace    : public Face< MyUsedTypes,   face::VertexRef, face::Normal3f, face::BitFlags, face::VFAdj, face::FFAdj > {};
class MyEdge    : public Edge< MyUsedTypes, edge::VertexRef, edge::BitFlags>{};
class MyMesh    : public tri::TriMesh< vector<MyVertex>, vector<MyEdge>, vector<MyFace>   > {};
void VcgOrient(ParticleSystem & p)
{
	    // cout<< "\n--------Orient Particle1--------------\n";
		MyMesh outMesh;
		outMesh.Clear();
		vcg::tri::Allocator<MyMesh>::AddVertices(outMesh, p._particles.size());
		for(unsigned int i=0;i< p._particles.size();++i)
		outMesh.vert[i].P()=vcg::Point3f(p._particles[i].pos.x,p._particles[i].pos.y,p._particles[i].pos.z);
		for(size_t i = 0; i < outMesh.vert.size(); ++i)
		outMesh.vert[i].index = i;
		vcg::tri::Allocator<MyMesh>::AddFaces(outMesh, p.nb_tri);
		for(size_t i = 0; i < p.nb_tri; i ++)
		{
			//cout<< "\n--------Orient Particle4--------------\n";
			 outMesh.face[i].V(0)=&(outMesh.vert[p._faces[i * 3]]);
			 outMesh.face[i].V(1)=&(outMesh.vert[p._faces[i * 3 + 1]]);
			 outMesh.face[i].V(2)=&(outMesh.vert[p._faces[i * 3 + 2]]);
		}
		
		bool oriented,orientable;
		//cout<< "\n--------Orient Particle5--------------\n";
		tri::UpdateTopology<MyMesh>::FaceFace(outMesh);
		//cout<< "\n--------Orient Particle5--------------\n";
		tri::Clean<MyMesh>::OrientCoherentlyMesh(outMesh,oriented,orientable);
		//assert(orientable);
		tri::Clean<MyMesh>::FlipMesh(outMesh);
		
		p._faces.clear();
		//cout<< "\n--------Orient Particle6--------------\n";
		for (size_t i = 0; i < outMesh.face.size(); i++)
		{
			for (int j = 0; j < 3; j++)
			{
                 	p._faces.push_back(outMesh.face[i].V(j)->index);
			}
		}
		
}*/
/*! \brief: import the vertices of the mesh file
*/
void MeshImporter::ImportVertices(std::ifstream & i_file, ParticleSystem & ps)
{
	std::string s;
	getline(i_file, s);

	int nb_verts = atoi(s.c_str());
	//cout << nb_verts;

	for (int i = 0; i < nb_verts; i++)
	{
		getline(i_file, s);

		float x, y, z;
		sscanf(s.c_str(), "%f %f %f", &x, &y, &z);
		//_part.push_back(Vector3(x, y, z)); // in order to orient
		ps.AddParticle(Vector3(x, y, z)); // ADD particles for particles system 
	     // which also contain add vertex, that fit my data structure
	}
	
}
/*! \brief: import the faces of the mesh file
*/
void MeshImporter::ImportFaces(std::ifstream & i_file, ParticleSystem & ps)
{
	std::string s;
	getline(i_file, s);

	int nb_faces = atoi(s.c_str());
	ps.nb_tri=nb_faces;
	//cout << nb_faces;
	for (int i = 0; i < nb_faces; i++)
	{
		getline(i_file, s);

		int idx[3];
		sscanf(s.c_str(), "%d %d %d", &idx[0], &idx[1], &idx[2]);
		for (int j = 0; j < 3; j++)
		{
			idx[j] -= 1;
			assert((idx[j] >= 0) && (idx[j] < ps._particles.size()));
		//	_indices.push_back(idx[j]); // in order to orient
		}
		// add tringle where in this function, we add in this fuction 3 edges
		ps._particles[idx[0]].isExternal=true;
		// add the external edges
		ps.AddTriangle(ps.GetVertex(idx[0]),
		ps.GetVertex(idx[1]),ps.GetVertex(idx[2]));
	     ps.AddPEdges(idx); // This is actually do nothing
	     ps.AddFace(idx); // again this actually do nothing
	} 
}
/*! \brief: import the tetrahderal of the mesh file
*/
void MeshImporter::ImportTetras(std::ifstream & i_file, ParticleSystem & ps)
{
	std::string s;
	getline(i_file, s);

	int nb_tetras = atoi(s.c_str());
	ps.nb_tetera=nb_tetras;
	for (int i = 0; i <nb_tetras; i++)
	{
		getline(i_file, s);

		int idx[4];
		sscanf(s.c_str(), "%d %d %d %d", &idx[0], &idx[1], &idx[2], &idx[3]);
		for (int j = 0; j < 4; j++)
			idx[j] -= 1;	// 0-based index
		
		ps.AddTetra(idx);// actually this really do nothing in this data structure 
	}
}
/*! \brief: import volumteric mesh that call
* vertices, faces and teterhderal
*/
void MeshImporter::Read(string & filename, ParticleSystem & ps)
{
	
	int tri=0;
	//cout<< "\n--------Before Loaded Tetrahedral--------------\n";
	const char *sptr = filename.c_str() ;
	std::ifstream i_file(sptr, std::ifstream::in || std::ifstream::out);
	if (!i_file)
	 {
      printf ("In Import mesh + ERROR! CANNOT OPEN '%s'\n",sptr);
      return;
      }
	std::vector<std::string> lines;
	if (i_file) {
		std::string s;
		while (getline(i_file, s))
		{
			lines.push_back(s);
			if (s == "Vertices")
				ImportVertices(i_file, ps);
			if (s == "Triangles")
			{
			ImportFaces(i_file, ps);
			tri= ps.triangles->Count();
			}
			if (s == "Tetrahedra")
			ImportTetras(i_file, ps);
		}
	}
	i_file.close();
	//cout<< "\n--------after Loaded Tetrahedral--------------\n";
	//ps.ComputeCentroid();
	ps.VcgOrient(ps); // orinet particles
	//cout<< "\n--------after Loaded Tetrahedral--------------\n";
	int num_faces,num_vertices,num_edges,tetnum;
	num_vertices =ps._particles.size();
	num_faces=ps._faces.size();
	num_edges=ps.edges->Count();
	tetnum=ps._tetras.size()/4;
	ps.Allocate(num_faces,num_vertices);
	//cout<< "--------Statistic About Tetrahedral----------\n";
	//std::cout <<"Vertices#:-------" <<ps._particles.size()<< std::endl;
	//std::cout <<"Faces#:-------"<<ps._faces.size()/3 << std::endl;
	//std::cout <<"Tetrahedral#:-------"<<  ps._tetras.size()/4<< std::endl;
	ps.DefineSystem(num_faces,
		num_vertices,
		num_edges,
		ps._particles,
		ps._faces,
		ps._edges,
		ps._tetras);
	//ps.CalculateFaceNormals();
	ps.ComputeNormals();
	//ps.computeNormals();
	//ps.SetGravity(Vector3(0,-200,0));
	/*cout<< "--------After Loaded Tetrahedral--------------\n";
	std::cout << "Vertices#:-------" <<ps._particles.size() << std::endl;
	std::cout << "Faces#:-------" << ps._faces.size()<< std::endl;
	std::cout << "Edges#:-------"<< ps.edges->Count()<< std::endl;
	std::cout << "My Professor Edges#:-------"<<ps._edges.size() << std::endl;
	std::cout << "Triangles#:-------"<< tri << std::endl;
	std::cout << "Tetrahedral#:-------"<< tetnum << std::endl;
	std::cout << "Triangles and Tetrahedral#:-------"<< tetnum + tri << std::endl;*/
}
