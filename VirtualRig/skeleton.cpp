#include "skeleton.h"
#include <limits>
//#define USE_PROFILE		// undef to disable profiling
#include "profile.h"
using namespace boost;
/*! \brief: Construct
*  
* skeleton not init
* no root yet and visible skeleton
*/
//  functor to compare two pairs <distance, *bone>
bool dist_bone_comp_func_ske(pair<float, CBone*> i, pair<float, CBone*> j) 
{ 
	return (i.first < j.first); 
}
CSkeleton::CSkeleton()
{
	inited = false;
	is_visible = true;
	root = -1;
	skel_average_distance=0;
}
/*! \brief: load
* load file male.skel as skeleton fit this model ( fitiing was done by
* Pinocchio MIT automatic rigging
* Find if there any prevouis link if there are, so its already exist
* otherwise link between parent and child link
*/
int CSkeleton::Load()
{
	PROFILE_SAMPLE("skeleton_load"); // profiling the code 
	link.clear(); // clear all link 
	vert.clear(); // clear all joints (vert for joint coordinates)
	bone_to_show_index = -1;
	// load file male.skel as skeleton fit this model ( fiting was done by
	// Pinocchio MIT automatic rigging
	cout << "skeleton-----------" << filename << endl;
	if (filename.empty())
		return -1;

	assert(!filename.empty());
	int result = 0;//vcg::tri::io::ImporterOBJ<CMesh>::E_NOERROR;

	ifstream stream(filename.c_str());
	if (stream.fail())
		return -1;//vcg::tri::io::ImporterOBJ<CMesh>::E_CANTOPEN;

	if(stream.eof()) 
		return result;
	char_separator<char> sep(" ");
	do
	{
		string line;
		getline(stream, line);
		if (!line.empty())
		{
			tokenizer<char_separator<char>> tokens(line, sep);
			vector<float> data;
			BOOST_FOREACH(string num, tokens)
			{
				double temp = ::atof(num.c_str());
				data.push_back(temp);
			}
			assert(data.size() == 5);

			CLink l;
			l.node = data[0];
			Point3f p = Point3f(data[1], data[2], data[3]);
			l.parent = data[4]; // start link based in parent
			link.push_back(l);
			vert.push_back(p);
			vert_rest.push_back(p);
			vert_prev.push_back(p);
			vert_bind.push_back(p);
		}
	}	
	while (!stream.eof());  

	// assign childs
	for (int i = 0; i < link.size(); i++)
	{
		if (link[i].parent == -1)	// skip the root(s)
			continue;

		int i_parent = FindLink(link[i].parent); // linking parent and child
		CLink & parent = link[i_parent];
		parent.childs.push_back(i);	// note: bone[i].node is equal to i
	}

	Init();
	return result;

}
/*! \brief: load new 
* load file male.skel as skeleton fit this model ( fitiing was done by
* Pinocchio MIT automatic rigging
* Find if there any prevouis link if there are, so its already exist
* otherwise link between parent and child link
*/
int CSkeleton::LoadNew(const QString & filePath)
{
	std::string Skel_name = filePath.toLocal8Bit().constData();
	Skel_name =Skel_name.append(".skel");
	PROFILE_SAMPLE("skeleton_load"); // profiling the code 
	link.clear(); // clear all link 
	vert.clear(); // clear all joints (vert for joint coordinates)
	bone_to_show_index = -1;
	// load file male.skel as skeleton fit this model ( fiting was done by
	// Pinocchio MIT automatic rigging
	filename=Skel_name;
	//cout << "skeleton-----------" << filename << endl;
	if (filename.empty())
		return -1;

	assert(!filename.empty());
	int result = 0;//vcg::tri::io::ImporterOBJ<CMesh>::E_NOERROR;

	ifstream stream(filename.c_str());
	if (stream.fail())
		return -1;//vcg::tri::io::ImporterOBJ<CMesh>::E_CANTOPEN;

	if(stream.eof()) 
		return result;
	char_separator<char> sep(" ");
	do
	{
		string line;
		getline(stream, line);
		if (!line.empty())
		{
			tokenizer<char_separator<char>> tokens(line, sep);
			vector<float> data;
			BOOST_FOREACH(string num, tokens)
			{
				double temp = ::atof(num.c_str());
				data.push_back(temp);
			}
			assert(data.size() == 5);

			CLink l;
			l.node = data[0];
			Point3f p = Point3f(data[1], data[2], data[3]);
			l.parent = data[4]; // start link based in parent
			link.push_back(l);
			vert.push_back(p);
			vert_rest.push_back(p);
			vert_prev.push_back(p);
			vert_bind.push_back(p);
		}
	}	
	while (!stream.eof());  

	// assign childs
	for (int i = 0; i < link.size(); i++)
	{
		if (link[i].parent == -1)	// skip the root(s)
			continue;

		int i_parent = FindLink(link[i].parent); // linking parent and child
		CLink & parent = link[i_parent];
		parent.childs.push_back(i);	// note: bone[i].node is equal to i
	}

	Init();
	return result;

}
/*! \brief: Save 
* save file the male.skel as skeleton fit this model 
*/
int CSkeleton::Save()
{
	if (filename.empty())
		return -1;

	assert(!filename.empty());
	int result = 0;//vcg::tri::io::ImporterOBJ<CMesh>::E_NOERROR;

	cout << "saving " << filename << endl;
	ofstream stream(filename.c_str());
	if (stream.fail())
		return -1;//vcg::tri::io::ImporterOBJ<CMesh>::E_CANTOPEN;

	for (int i = 0; i < bone.size(); i++)
	{
		if (bone[i].parent == -1)
			stream << bone[i].node << " " << vert[bone[i].node][0] << " " << vert[bone[i].node][1] << " " << vert[bone[i].node][2] << " " << bone[i].parent << endl;
		stream << bone[i].child << " " << vert[bone[i].child][0] << " " << vert[bone[i].child][1] << " " << vert[bone[i].child][2] << " " << bone[i].node << endl;
	}

	return 0;
}
/*! \brief: Reset 
* reset initialization with local coordinate 
*/
void CSkeleton::bindpose()
{
	for (int i = 0; i < vert.size(); i++)
	{
		vert[i] = vert_bind[i];
		vert_prev[i] = vert_bind[i];
	}

	//InitLocalFrame();
}
/*! \brief: Reset 
* reset initialization with local coordinate 
*/
void CSkeleton::Reset()
{
	for (int i = 0; i < vert.size(); i++)
	{
		vert[i] = vert_rest[i];
		vert_prev[i] = vert_rest[i];
	}

	InitLocalFrame();
}
/*! \brief: linking the vertices / Joints
* which link is bone 
*/
void CSkeleton::Init()
{
	dir_rot.SetZero();

	// build the bones
	bone.clear();
	bbox.SetNull();
	bone_to_show_index = -1;

	for (int i = 0; i < link.size(); i++)
	{
		CLink & l = link[i];
		for (int j = 0; j < l.childs.size(); j++)
		{
			CBone b; 
			b.node = i;
			b.parent = l.parent;
			b.child = l.childs[j];
			b.L.SetIdentity(); /// local coordinate I matrix
			b.rdist = (vert[b.child] - vert[b.node]).Norm();
			b.index = bone.size();

			bone.push_back(b);
		}
		bbox.Add(vert[i]);					// init bounding box
	}

	// for every bone, assign child-bones 
	for (int i = 0; i < bone.size(); i++)
	{
		CBone & b = bone[i];

		for (int j = i + 1; j < bone.size(); j++)
			if (bone[j].node == b.child)
				b.childs.push_back(&bone[j]);
	}

	// init the movable vector
	movable.clear();
	movable.insert(movable.begin(), vert.size(), false);

	InitLocalFrame();

#if 0
	// assign each vertex of the mesh to a local reference frame of the matrix
	if (mesh)
		for (CMesh::VertexIterator vi = mesh->vert.begin(); vi != mesh->vert.end(); vi++)
		{
			Point3f & vpos = vi->P();
			float _min = std::numeric_limits<float>::max();
			int index = -1;

			for (int i = 0; i < bone.size(); i++)
			{
				// find the bone nearest to the current vertex
				Point3f & s = vert[bone[i].node];
				Point3f & d = vert[bone[i].child];
				Segment3f seg_bone(s, d);
				float dist;

				Point3f clos;
				//dist = SquaredDistance<float>(seg_bone, vpos);

				Point3f	 dir = (d - s).Normalize();
				float h = dir * (vpos - s);
				if ((h <= 0.0) || h > seg_bone.Length())
					continue;	// this bone is discarded

				SegmentPointSquaredDistance<float>( seg_bone, vpos, clos, dist); 
				if (dist < _min)
				{
					_min = dist;
					index = i;
				}
			}

			if (index != -1)
				bone[index].vp.push_back(&*vi);
		}
#endif


}
/*! \brief: Assign 
* specific particle to skeleton
*/
/*void CSkeleton::AssignParticle(ParticleSystem * _skin)
{
//pskin = _skin ;
//ParticleNearestBone();
}*/
/*! \brief: Assign 
* specific mesh to skeleton
*/
void CSkeleton::Assign(CMesh *physics)
{
	//cout << " here we are !! ";
	mesh=physics; // define skin in physics
	//cout << mesh->skin._particles.size();
	ParticleNearestBone();
	//VertexNearestBone();
}
/*! \brief: find for each particle nearest joint 
*  with skeleton 
**** review the output all
*/
void CSkeleton::ParticleNearestBone()
{
//cout << "particle nearest bone assginment ... \n";
	int n_bones = 1;
	//cout << "mesh vertices size" <<phys._particles.size();
     // for each particle v
	for (int i = 0; i <mesh->skin._particles.size(); i++)
	{	
		vector<pair<float, CBone*>> distances;
		Particle & p =mesh->skin._particles[i];
          //cout << "\nVertex ========>>>" << p.pos.x << " " << p.pos.y<< " " << p.pos.z<< endl;
		//for each bone b
		vector<CBone>::iterator bi = bone.begin();
		for (; bi != bone.end(); bi++)
		{
			CBone & bone = *bi;	
			// get the corresponding segment {vert here we are getting bone in Skeleton}
			Point3f & s = vert[bone.node];      // node[joint] O=========O child[joint] // actual
			Point3f & d = vert[bone.child];     //////////////   bone  ///////////////
			Segment3f seg_bone(s, d); // our segment bone
			//cout << "sbone ===" << s.X() << " " << s.Y() << "  " << s.Z() <<endl ; 
			//cout << "dbone ===" << d.X() << " " << d.Y() << "  " << d.Z() <<endl; 

			//	compute distance d between v in the mesh  and segment  seg_bone

			Point3f closest;
			Point3f par;
			par.X()=p.pos.x;
			par.Y()=p.pos.y;
			par.Z()=p.pos.z;
			float dist;
			// compute the sqaured distance between seg and v return point and value
			SegmentPointDistance<float>(seg_bone, par, closest, dist);
			//cout<< " closest " << closest.X() << "   " << closest.Y() << "    " << closest.Z()<< endl;
			//cout<< " closest distance" << dist <<endl; 
			distances.push_back(pair<float, CBone*>(dist, &bone)); // keep distance and bone
	
		}

		sort<vector<pair<float, CBone*>>::iterator>(	distances.begin(),  
			distances.end(), 
			dist_bone_comp_func_ske);  // sort all bone based on there distance from vertex

		// get the n bones with smallest distance
		for (int j = 0; j < n_bones; j++)
		{
			assert(distances.size() >= n_bones);
			int vertexindex = p.index;
			int boneindex= 0.1f/distances[j].second->index;
			int boneindexchild= distances[j].second->child;
			//////////// assign nearest bone to vertex /////////////
			vertassignbone.push_back(make_pair(vertexindex,boneindex));
			//////////////////// *************assgining weight *************** /////////////////
			Particle::CPartWeight bw;
			bw.bone = distances[j].second; // assgin for each particle nearest bone
			bw.distance = distances[j].first; // inital or reset distance
			//cout<< " bw distance" << bw.distance <<endl; 
			bw.weight = 1.0f/distances[j].first; // inital or reset distance with neatest bone
			/////////////////////////////////////////////////////////////////////
			p.nearestbone=bw; // nearest bone
			p.PushPartBoneWeight(bw); ////// add weight
			//p.oldPos=p.pos;
		}
	}
	AssginParticlesWeights();
	StretchBoneConstraint();
	
}
void CSkeleton::AssginParticlesWeights()
{    
	//Normalize weights
	//cout << " inside find AssginParticlesWeights------------------? " << mesh->skin._particles.size()  ; 
	for (vector<Particle>::iterator pi = mesh->skin._particles.begin(); pi != mesh->skin._particles.end(); pi++)
	pi->NormalizeParWeights();
	
	//Assign coordinates in the local frame of the bone to which the vertex belongs to
	
	Update();
	for (vector<Particle> ::iterator pi = mesh->skin._particles.begin(); pi != mesh->skin._particles.end(); pi++)
	{
		for (vector<Particle::CPartWeight> ::iterator it= mesh->skin._particles[pi->index].particle_weight.begin(); 
			it !=mesh->skin._particles[pi->index].particle_weight.end(); it++)
	{

			//Particle::particle_weight.
			Particle::CPartWeight & bw = *it;
			CBone & bone = *(bw.bone);
			Point3f vpos;
			vpos.X()=pi->pos.x;
			vpos.Y()=pi->pos.y;
			vpos.Z()=pi->pos.z;
			Point3f v = vpos - vert[bone.node];
			float lx = v * bone.x;
			float ly = v * bone.y;
			float lz = v * bone.z;
			bw.lcoord = Vector3(lx, ly, lz); // from local coordinate
		}
	}
	
}
/*! \brief: the constraint moves the particle in order to
mantain the initial rest length.
*/
void CSkeleton ::StretchBoneConstraint()
{
	int w=0;
// having the particle where each particle have the nearest joint 
// the child [ so we have segment]
// for loop through particle
for (vector<Particle> ::iterator pi = mesh->skin._particles.begin(); pi != mesh->skin._particles.end(); pi++)
	{
              
		     /*if(w<190)
			{*/
			Vector3 delta,cdelta;
			Point3f vpos,nearestpoint;
			vpos.X()=pi->pos.x; // particle postion
			vpos.Y()=pi->pos.y;
			vpos.Z()=pi->pos.z; 
			//cout << " particle " << vpos.X() << " " << vpos.Y() << " " << vpos.Z();
			Particle::CPartWeight & nb = pi->nearestbone; // nearest bone
			float rest =nb.distance;
			//cout << "\n rest ============" << rest;
			Segment3f seg_bone(vert[nb.bone->node], vert[nb.bone->child]); // our segment bone
		     SegmentPointDistance<float>(seg_bone, vpos, nearestpoint, rest);
			//cout << "\n rest ============" << rest;
			delta.x= nearestpoint.X();  // which is P 
			delta.y= nearestpoint.Y();
			delta.z= nearestpoint.Z();
			nb.nearpoint=delta;
			cdelta.x= nearestpoint.X()-pi->pos.x;
			cdelta.y= nearestpoint.Y()-pi->pos.y;
			cdelta.z= nearestpoint.Z()-pi->pos.z;
			nb.mdelta=cdelta;
			// finding tp
			float tp;
			tp=(nearestpoint - vert[nb.bone->child]).Norm()/(vert[nb.bone->node]-vert[nb.bone->child]).Norm();
			nb.tp=  tp;

			//cout <<  "\n the tp " << tp << endl;
			Constraint c(BONESTRETCH, rest,mesh->skin.bone_stiffness);
			Particle & p0 = mesh->skin._particles[pi->index]; 
			//cout << " distance in the skel " << skel_average_distance;
			if (nb.distance<skel_average_distance)
				p0.Pstretch_stiffness=mesh->skin.stretch_stiffness;
			else
				p0.Pstretch_stiffness=mesh->skin.stretch_stiffness;
			//cout << p0.pos.x << " " << p0.pos.y << p0.pos.z << endl;
			c.ref_parts.push_back(& p0);// actual particle
			c.ref_bones.push_back(&nb); // index of the nearest bone
		     mesh->skin.AddConstraint(c);
			/*w++;
			}**/
}
mesh->skin.bone_constriant= mesh->skin._constraints.size()-(mesh->skin.stretch_constriant + mesh->skin.bend_constriant+ mesh->skin.tet_constriant);
}
/*! \brief: find for each vertex nearest joint 
*  with mesh and skeleton 
*/
void CSkeleton::VertexNearestBone()
{
	cout << "mesh nearest bone assginment ... \n";
	int n_bones = 1;
	cout << "mesh vertices size" <<mesh->vert.size() ;

	// for each vertex v
	for (int i = 0; i < mesh->vert.size(); i++)
	{	
		vector<pair<float, CBone*>> distances;
		CVertex & v = mesh->vert[i];
		//cout << "Vertex ========>>>" << v.P().X() << " " << v.P().Y() << " " << v.P().Z() << endl;
		//for each bone b
		vector<CBone>::iterator bi = bone.begin();
		for (; bi != bone.end(); bi++)
		{
			CBone & bone = *bi;	
			// get the corresponding segment {vert here we are getting bone in Skeleton}

			Point3f & s = vert[bone.node];      // node[joint] O=========O child[joint] // actual
			//cout << " is it index for bone " << bone.node << endl;
			//cout << " is it index for source" << bone.child << endl;

			Point3f & d = vert[bone.child];     //////////////   bone  ///////////////
			Segment3f seg_bone(s, d); // our segment bone
			//cout << "sbone ===" << s.X() << " " << s.Y() << "  " << s.Z() <<endl ; 
			//cout << "dbone ===" << d.X() << " " << d.Y() << "  " << d.Z() <<endl; 

			//	compute distance d between v in the mesh  and segment  seg_bone

			Point3f closest;
			float sqr_dist;
			// compute the sqaured distance between seg and v return point and value
			SegmentPointSquaredDistance<float>(seg_bone, v.P(), closest, sqr_dist);

			//cout<< " closest " << closest.X() << "   " << closest.Y() << "    " << closest.Z()<< endl;
			//cout<< " closest distance" << sqr_dist <<endl; 

			distances.push_back(pair<float, CBone*>(sqr_dist, &bone)); // keep distance and bone
		}

		sort<vector<pair<float, CBone*>>::iterator>(	distances.begin(),  
			distances.end(), 
			dist_bone_comp_func_ske);  // sort all bone based on there distance from vertex

		// get the n bones with smallest distance
		for (int j = 0; j < n_bones; j++)
		{
			assert(distances.size() >= n_bones);
			int vertexindex = v.Index();
			int boneindex= distances[j].second->index;
			//////////// assign nearest bone to vertex /////////////
			vertassignbone.push_back(make_pair(vertexindex,boneindex));
			// cout << "nearest vertex" << vert[boneindex].X()  << "  " << vert[boneindex].Y() << "  " << vert[boneindex].Z();
			// cout<< mesh->vert[vertexindex].P().X() << "  " << mesh->vert[vertexindex].P().Y() 
			//	  << " " << mesh->vert[vertexindex].P().Z() ;
			//////////////////// *************assgining weight *************** /////////////////
			CVertex::CBoneWeight bw;
			bw.bone = distances[j].second;
			bw.weight = 1.f / distances[j].first;
			v.PushBoneWeight(bw);

		}
	}
	AssginWeights();
}
void CSkeleton::AssginWeights()
{

	// normalize weights
	for (CMesh::VertexIterator vi = mesh->vert.begin(); vi != mesh->vert.end(); vi++)
		vi->NormalizeWeights();

	// assign coordinates in the local frame of the bone to which the vertex belongs to
	Update();
	for (CMesh::VertexIterator vi = mesh->vert.begin(); vi != mesh->vert.end(); vi++)
	{
		for (vector<CVertex::CBoneWeight>::iterator it = vi->bone_weight.begin(); it != vi->bone_weight.end(); it++)
		{
			CVertex::CBoneWeight & bw = *it;
			CBone & bone = *(bw.bone);
			Point3f v = vi->P() - vert[bone.node];

			float lx = v * bone.x;
			float ly = v * bone.y;
			float lz = v * bone.z;

			bw.lcoord = Point3f(lx, ly, lz);
		}
	}

}
/*! \brief:  
* initialization with local coordinate for each bone
*/
void CSkeleton::InitLocalFrame()
{
	// init local coordinate frame
	for (int i = 0; i < bone.size(); i++)
	{
		Point3f & s = vert[bone[i].node]; // parent joint
		Point3f & d = vert[bone[i].child]; // child Joint
		// is this the bone direction??? / lenght
		bone[i].z = (d - s).Normalize();
		bone[i].z_prev = bone[i].z;
		bone[i].z_rest = bone[i].z;

		if ((bone[i].z * Point3f(0, 0, 1)) > 0)
			bone[i].y = Point3f(0, 1, 0);
		else
			bone[i].y = Point3f(0, -1, 0);

		bone[i].x = bone[i].y ^ bone[i].z;
		bone[i].x.Normalize();
		bone[i].y = bone[i].z ^ bone[i].x;
		bone[i].y.Normalize();

		bone[i].L.SetColumn(0, bone[i].x);
		bone[i].L.SetColumn(1, bone[i].y);
		bone[i].L.SetColumn(2, bone[i].z);

		Point3f v = d - s;

		float lx = v * bone[i].x;
		float ly = v * bone[i].y;
		float lz = v * bone[i].z;

		//		bone[i].lchild = Point3f(lx, ly, lz);
	}
}
/*! \brief:  
* update local coordinate for each bone
*if bone rotate or translate so the direction rotation 
* each bone can rotate with specific anagle
*/
void CSkeleton::UpdateLocalFrame()
{
	if (!inited)
		return;

	if (dir_rot != Point3f(0, 0, 0))
	{
		vcg::Plane3f plane;
		plane.Init(vert[bone[1].node], dir_rot);
		vert[bone[1].child] = plane.Projection(vert[bone[1].child]);
	}

	for (int i = 0; i < bone.size(); i++)
	{
		Point3f & s = vert[bone[i].node];
		Point3f & d = vert[bone[i].child];

		bone[i].z = (d - s).Normalize();

		if (bone[i].z_prev == bone[i].z)
			continue;

		float angle = Angle(bone[i].z_prev, bone[i].z);
		Point3f axis = (bone[i].z_prev ^ bone[i].z).Normalize();

		Matrix33f mrot;
		mrot.SetRotateRad(angle, axis);

		bone[i].y = mrot * bone[i].y;
		bone[i].x = mrot * bone[i].x;

		bone[i].L.SetColumn(0, bone[i].x);
		bone[i].L.SetColumn(1, bone[i].y);
		bone[i].L.SetColumn(2, bone[i].z);

		bone[i].z_prev = bone[i].z;

		////// forward kinematics
		//{
		//	Matrix33f mrot;
		//	mrot.SetRotateRad(-angle, axis);
		//for (int j = 0; j < bone[i].childs.size(); j++)
		//	RotateBone(mrot, bone[i].childs[j]);
		//}
		//float angle_r = Angle(bone[i].z, bone[i].z_rest);
		//cout << "bone " << i << ": " << angle_r << " rad " <<  math::ToDeg(angle_r) << " deg" << endl;
	}
}
/*! \brief:  
* Param : rotation matrix , parent bone , child bones
* where child with rotate buy muliple by rotate matrix
*/
void CSkeleton::RotateBone(const Matrix33f & mrot, const CBone* pParent, CBone* pBone)
{
	for (int i = 0; i < pBone->childs.size(); i++)
	{
		CBone* pChild = pBone->childs[i];
		vert[pChild->node] -= vert[pParent->node]; // unlink with parent
		vert[pChild->node] =  mrot * vert[pChild->node]; // multiple with child
		vert[pChild->node] += vert[pParent->node]; // link again

		RotateBone(mrot, pParent, pChild); // recursively do it for childs 
	}
}
/*! \brief:  
* Param : index of the bone you went to rotate , angle , which axis
* actually I dont know
*/
void CSkeleton::RotateBone(int index_bone, float angle, Point3f axis_rot)
{
	assert(index_bone < bone.size());

	bone[index_bone].z = bone[index_bone].z_rest;

	Matrix33f mrot;
	mrot.SetRotateRad(angle, axis_rot);

	bone[index_bone].z = mrot * bone[index_bone].z;

	Point3f & s = vert[bone[index_bone].node];
	Point3f & d = vert[bone[index_bone].child];

	float length = (d - s).Norm();
	vert_prev[bone[index_bone].child] = vert[bone[index_bone].child];
	vert[bone[index_bone].child] = vert[bone[index_bone].node] + bone[index_bone].z * length;

	//	Update();
}
//void CSkeleton::SetMovable(const CBone & _bone)
//{
//	for (int i = 0; i < _bone.childs.size(); i++)
//	{
//		movable[_bone.childs[i]->node] = true;
//		SetMovable(*_bone.childs[i]);
//	}
//		int FindLink(int node)
//
//}
/*-----------------------------------------------------*/
/*! \brief:  
* Param : index of the bone , find the link of this bone
* set the parent and child as movable bone
*/
void CSkeleton::SetMovable(int _i)
{
	int index = FindLink(_i);
	assert(index != -1);
	movable[link[index].node] = true;
	//	printf("CSkeleton::SetMovable: %d\n", link[index].node);

	for (int i = 0; i < link[index].childs.size(); i++)
		SetMovable(link[index].childs[i]);
}
/*! \brief:  
* update local coordinate for each child bone
*if bone rotate or translate so the direction rotation 
*/
void CSkeleton::InitLocalChilds()
{
	for (int i = 0; i < bone.size(); i++)
	{
		//	for (int j = 0; j < bone[i].childs.size(); j++)
		{
			const Point3f & s = vert[bone[i].node];
			const Point3f & d = vert[bone[i].child];

			const Point3f v = d - s;

			float lx = v * bone[i].L.GetColumn(0);
			float ly = v * bone[i].L.GetColumn(1);
			float lz = v * bone[i].L.GetColumn(2);

			//bone[i].lchilds.push_back(Point3f(lx, ly, lz));
			// update the child coordinate frame
			bone[i].lchild = Point3f(lx, ly, lz);

			Point3f p = vert[bone[i].node] + bone[i].L * bone[i].lchild;
			assert((p - d).Norm() < 0.0001);

		}
	}
	inited = true;

}

void CSkeleton::Update()
{
	if (!inited)
		return;

	for (int i = 0; i < bone.size(); i++)
	{
		//		for (int j = 0; j < bone[i].childs.size(); j++)
		{
			vert[bone[i].child] = vert[bone[i].node] + bone[i].L * bone[i].lchild;
		}
	}

	//	return;

	// mark all the verts as not movable
	for (int i = 0; i < movable.size(); i++)
		movable[i] = false;

	// identify the index of the changed vertex (if any)
	// mark the bone.node and all the children as movable (recursively)

	for (int i = 0; i < bone.size(); i++)
	{
		if (vert[bone[i].node]  != vert_prev[bone[i].node]) 
		{
			movable[bone[i].node] = true;
			//			SetMovable(bone[i].node);
		}

		if (vert[bone[i].child] != vert_prev[bone[i].child])	
			movable[bone[i].child ] = true;
	}

	int nbIters = 20;
	for (int iters = 0; iters < nbIters; iters++)
	{
		for (int i = 0; i < bone.size(); i++)
		{
			Point3f & s = vert[bone[i].node];
			Point3f & d = vert[bone[i].child];

			Point3f delta = s - d;
			float dist = delta.Norm();

			double diff = (dist - bone[i].rdist) / dist;

			//			if (movable[bone[i].node])
			s -= delta * 0.5 * diff * 1.f;//stiffness;

			//			if (movable[bone[i].child])
			d += delta * 0.5 * diff * 1.f;//stiffness;
		}

#if 0
		// do not move the root
		for (int i = 0; i< vert.size(); i++)
		{
			if (link[i].parent == -1)
			{
				vert[i] = vert_rest[i];
				for (int j = 0; j < link[i].childs.size(); j++)
				{
					int id_child = link[i].childs[j];
					vert[id_child] = vert_rest[id_child];
				}
			}
		}
#endif
	}

	// update L for each bone
	//	UpdateLocalFrame();

	for (int i = 0; i < vert.size(); i++)
		vert_prev[i] = vert[i];
	//StretchBoneConstraint();

}


int CSkeleton::FindLink(const Point3f & p)
{
	for (int i = 0; i < vert.size(); i++)
		if ((p - vert[i]).SquaredNorm() < 0.0001f)
			return i;

	return -1;
}
/* link between joint and joint which in order to establish bone
*/
void CSkeleton::AddLink(const Point3f & p)
{
	int index = FindLink(p);
	if (index != -1)	// the link already exists
	{
		cout << "link " << index << "already exists" << endl;
		return;
	}

	CLink l;
	l.parent = -1;
	vert.push_back(p);
	vert_rest.push_back(p);
	vert_prev.push_back(p);
	l.node = vert.size() - 1;

	link.push_back(l);
}
/* link between bone parent and  child as edge
bone  ------------- bone
*/
void CSkeleton::AddLink(const Point3f & s, const Point3f & d)
{
	AddLink(s);	// add the links, do nothing is they already exists
	AddLink(d);

	int source = FindLink(s); 
	int dest = FindLink(d);
	assert(dest != -1);

	link[source].childs.push_back(dest);
	link[dest].parent = source;

	Init();
}

void CSkeleton::IncBoneToShow()
{
	bone_to_show_index = (bone_to_show_index + 1) % bone.size();
	printf("showing bone: %d\n", bone_to_show_index);
}

void CSkeleton::DecBoneToShow()
{
	bone_to_show_index--;
	if (bone_to_show_index < 0)
		bone_to_show_index = bone.size() - 1;
	printf("showing bone: %d\n", bone_to_show_index);
}
/* render and show the skeleton 
/  each joint consider as vertex here 
/  and each linking which are the actual bone
*/
void CSkeleton::Draw(bool is_opaque)
{
	//if (!inited)
	//	return;

	if (!is_visible)
		return;

	glDisable(GL_LIGHTING);
	if (!is_opaque)
	{
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
	}
	glBegin(GL_LINES);
	for (int i = 0; i < bone.size(); i++)
	{
		bone[i].x = bone[i].L.GetColumn(0);
		bone[i].y = bone[i].L.GetColumn(1);
		bone[i].z = bone[i].L.GetColumn(2);

		Point3f & s = vert[bone[i].node];
		Point3f & d = vert[bone[i].child];
		glColor4f(1, 1, 1, 1);	// joint
	//	if (i == bone_to_show_index)
		//	glColor4f(1, 0, 1, 1);	// bone currently showed

		glVertex(s);
		glVertex(d);
		glColor4f(0, 0, 1, 1);	// z axis
		glVertex(s);
		glVertex(s + bone[i].z * bbox.Diag() * 0.05);
		glColor4f(1, 0, 0, 1);	// x axis
		glVertex(s);
		glVertex(s + bone[i].x * bbox.Diag() * 0.05);
		glColor4f(0, 1, 0, 1);	// y axis
		glVertex(s);
		glVertex(s + bone[i].y * bbox.Diag() * 0.05);
	}
	glEnd();

#if 0
	// draw the vertices belonging to the last bone
	glPointSize(4.f);
	glBegin(GL_POINTS);
	if ((bone_to_show_index != -1) && !bone.empty())
	{
		for (int i = 0; i < bone[bone_to_show_index].vp.size(); i++)
		{
			Color4f c;
			float min = 0;
			float max = 1;
			c.ColorRamp(min, max, 1 - bone[bone_to_show_index].w[i]);

			glColor(c);
			Point3f & s = bone[bone_to_show_index].vp[i]->P();
			glVertex(s);
		}
	}
#endif
	glEnd();



	glPointSize(8.f);
	glBegin(GL_POINTS);
	glColor4f(1, 0, 0, 1);
	for (int i = 0; i < link.size(); i++)
	{
		Point3f & s = vert[i];
		glVertex(s);
		//glPushMatrix();
		//glTranslate(s);
		//glutSolidSphere(2, 10, 10);
		//glPopMatrix();
	}
	if (vert.size() > 0)
	{
		glColor4f(0, 1, 1, 1);
		Point3f & s = vert[0];	// root
		glVertex(s);
	}
	glEnd();
	glBegin(GL_LINES);
	glLineWidth(2.3);
	for (int i = 0; i < bone.size(); i++)
	{
	
		Point3f & s = vert[bone[i].node];
		Point3f & d = vert[bone[i].child];
		glColor3f(0.8, 0.8, 0.8);	// joint
		glVertex(s);
		glVertex(d);
	}
	glEnd();
	if (!is_opaque)
	{
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
	}
	glEnable(GL_LIGHTING);
}