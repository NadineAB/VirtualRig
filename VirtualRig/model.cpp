#  pragma warning(disable:4002)
#include "skeleton.h"
#include "model.h"
#include "timer.h"
//#define USE_PROFILE		// undef to disable profiling
#include "profile.h"
int CModel:: count=0;
bool dist_bone_comp_func_skeP(pair<float, CBone*> i, pair<float, CBone*> j) 
{ 
	return (i.first < j.first); 
}
CModel::CModel()
{
	// used when a xml archive is created from scratch
	name = "male";
	skeleton = NULL;
	lines_visible = false;
	max_distance=1;
	average_distance=0; // average distance
	//cout << "here inside model " ;


#if 0

	//anim_dir = ".\\morphs\\S001-108";
	//anim_dir = ".\\morphs\\S002-009";
	//anim_dir = ".\\morphs\\S001-117";

#endif

}
///////////////////////////// Performance Profiling \\\\\\\\\\\\\\\\\\\\\\\\\  
/**************************************************************************** 
/* this first method called in this prototype ( so order to skin the code 
performance which : 
1. First load 3d Mesh ( body , eyes and  ect)
a. read xml file 
b. CMesh::Init() 
c. CMesh::DefineParticleSystem ( add particle, add faces and add edges)
d. ParticleSystem:: DefineParticleSystem
e. ParticleSystem::DefineConstraints
f. ParticleSystem :: SetGravity
2. Load skeleton ( which work with bvh and assign the weight):
Have alot of part but we considering just load skeleton anf bvh 
without caring about assigning weight
/**************************************************************************** */
string CModel::Init()
{     
	/////// Profile INIT MODLE ///////
	PROFILE_SAMPLE("INIT_MODEL");
	//cout << "model init" << endl;
	string err; 
	float t0;
	if (!anim_dir.empty())
	{
		if (meshes.size() > 0)
			meshes[0]->LoadMorphs(anim_dir, "skin");
		if (meshes.size() > 1)
			meshes[1]->LoadMorphs(anim_dir, "eye_left");
		if (meshes.size() > 2)
			meshes[2]->LoadMorphs(anim_dir, "eye_right");
		if (meshes.size() > 3)
			meshes[3]->LoadMorphs(anim_dir, "teeth_lower");
		if (meshes.size() > 4)
			meshes[4]->LoadMorphs(anim_dir, "teeth_upper");
		if (meshes.size() > 5)
			meshes[5]->LoadMorphs(anim_dir, "tongue");
	}
	//else
	//	cout << "no face morphs" << endl;
	/*
	///////////////////////////////// 1. End \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
	*/
	/*
	///////////////////////////// 2. PROFILE LOAD MESHES \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\
	*/
	if (skeleton)
	{   
		{
			PROFILE_SAMPLE("Load Skeleton::");
			skeleton->Load();

		}
	}
	skeleton->Assign(meshes[0]);
	//cout << " after finding the nearest bone ";
	//cout<< skeleton->vertassignbone.size();
	////////////////////////////////// 2. END \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*/

	if (!skeleton)		// ******** RETURN HERE IS NO SKELETON IS INITED
		return err;
	else 
		meshes[0]->skin.ske=skeleton;
	return err;

	//********** update the weight for skeleton  *****************//	
	// compute the rotation of the arm to reach the target position
	// get the position of the last vertex of the arm on the xpositive axis

	cout << "finding rotation direction ... ";
	t0 = GetRealTime();
	float xmax0 = meshes[0]->bbox.min[0];
	int index0 = -1;
	for (int i = 0; i < meshes[0]->vert.size(); i++)
	{
		if (meshes[0]->vert[i].P()[0] > xmax0)
		{
			xmax0 = meshes[0]->vert[i].P()[0];
			index0 = i;
		}
	}

	float zmax = meshes.back()->bbox.min[2];
	int index1 = -1;
	for (int i = 0; i < meshes.back()->vert.size(); i++)
	{
		if (meshes.back()->vert[i].P()[2] > zmax)
		{
			zmax = meshes.back()->vert[i].P()[2];
			index1 = i;
		}
	}

	Point3f p0 = skeleton->vert[skeleton->bone[1].node];	// position of the elbow joint

	Point3f v1 = meshes[0]->vert[index0].P() - p0;
	Point3f v2 = meshes.back()->vert[index1].P() - p0;

	skeleton->dir_rot =  (v2 ^ v1).Normalize();
	skeleton->max_angle_rot = math::ToDeg(Angle(v2, v1));

	cout << GetRealTime() - t0 << endl;

	skeleton->Update();

	return err;

	// clamp the weights in mesh 0 
	cout << "adding bones and clamping weights between 0 and 1 ... ";
	t0 = GetRealTime();
	for (CMesh::VertexIterator vi = meshes[0]->vert.begin(); vi != meshes[0]->vert.end(); vi++)
	{
		if (vi->bone_weight.size() == 2)
		{
			if (vi->bone_weight[0].weight > 0.5)
			{
				vi->bone_weight[0].weight = 1;
				vi->bone_weight[1].weight = 0;
			}
			else
			{
				vi->bone_weight[0].weight = 0;
				vi->bone_weight[1].weight = 1;
			}
		}

	}
	cout << GetRealTime() - t0 << endl;

	for (CMesh::VertexIterator vi = meshes[0]->vert.begin(); vi != meshes[0]->vert.end(); vi++)
		assert((vi->bone_weight.size() == 0) || (vi->bone_weight.size() == 2));

	skeleton->RotateBone(1, math::ToRad(-81.f), skeleton->dir_rot);


#if 1
	// shot rsys to find negative weights
	cout << "finding not moving vertices  ... ";
	t0 = GetRealTime();
	vector<bool>  is_in_mesh_base;
	for (int i = 0; i < meshes[0]->vert.size(); i++)
	{
		bool res = false;
		const Point3f & p0 = meshes[0]->vert[i].P() ;
		const Point3f & p1 = meshes.back()->vert[i].P();
		if ((p0 - p1).SquaredNorm() < 0.01)
			res = true;

		is_in_mesh_base.push_back(res);
	}
	cout << GetRealTime() - t0 << endl;
#endif
	cout << "finding matching between source and target mesh  ... begin" << endl;
	t0 = GetRealTime();
	vector<float> weights;
	vector<Point3f> targets;
	for (int i = 0; i < meshes[0]->vert.size(); i++)
	{
		CVertex & v = meshes[0]->vert[i];
		float t = -1;
		Point3f pt;

		// figure out if the vertex belongs to bone 0 or not
		if ((v.bone_weight.size() > 0) && !is_in_mesh_base[v.Index()])
			if (((v.bone_weight[0].bone->node == 0) && (v.bone_weight[0].weight == 1)) ||
				((v.bone_weight[1].bone->node == 0) && (v.bone_weight[1].weight == 1)))
			{
				int i0, i1;
				if (v.bone_weight[0].bone->node == 0)
				{
					i0 = 0;
					i1 = 1;
				}
				else
				{
					i0 = 1;
					i1 = 0;
				}
				CBone & bone0 = *(v.bone_weight[i0].bone);
				CBone & bone1 = *(v.bone_weight[i1].bone);
				const Point3f & p0 = skeleton->vert[bone0.node] + bone0.L * v.bone_weight[i0].lcoord;
				const Point3f & p1 = skeleton->vert[bone1.node] + bone1.L * v.bone_weight[i1].lcoord;
				Ray3f ray1(p0, (p1 - p0).Normalize() * -1);
				Ray3f ray2(p0, (p1 - p0).Normalize() * +1);
				Ray3f ray;

				float t1 = -1, t2 = -1;
				//vcg::tri::DoRay<CMesh, CMesh::SpatialGridType>(	*meshes.back(), 
				//meshes.back()->spatial_grid, 
				//ray1, 10000, t1);

				//vcg::tri::DoRay<CMesh, CMesh::SpatialGridType>(	*meshes.back(), 
				//	meshes.back()->spatial_grid, 
				//	ray2, 10000, t2);

				int sign = +1;
				t = t2;
				ray = ray2;
				if (fabs(t1) < fabs(t2))
				{
					t = t1;
					ray = ray1;
					sign = -1;
				}

				if (fabs(t) >= .5)
					t = -1;

				//				cout << i << ": " << t << " t1: " << t1 << " t2: " << t2 << endl;

				pt = ray.Origin() + ray.Direction() * t; // target point

				if ((t != -1) && (fabs(t) < .5))
				{
					dpoints.push_back(pt);
					dlines.push_back(pair<Point3f, Point3f>(p0, pt));
				}

				t = sign * (pt - p0).Norm() / (p1 - p0).Norm();
			}
			weights.push_back(t);
			targets.push_back(pt);
	}
	cout << "finding matching between source and target mesh  ... end in " << GetRealTime() - t0 << endl;

#if 1	
	cout << "assigning weights  ... ";
	t0 = GetRealTime();

	// for some reason, sometime  t value is wrong, this should be investigated
	// detect here the wrong t by comparing the result of the convex combination with pt
	for (CMesh::VertexIterator vi = meshes[0]->vert.begin(); vi != meshes[0]->vert.end(); vi++)
	{
		Point3f pos(0, 0, 0);
		if (vi->bone_weight.empty())
			continue;

		vector<CVertex::CBoneWeight>::iterator it = vi->bone_weight.begin();
		for (; it != vi->bone_weight.end(); it++)
		{
			CVertex::CBoneWeight & bw = *it;
			CBone & bone = *(bw.bone);

			float weight;
			if (bone.node == 0)
				weight = 1 - weights[vi->Index()];

			if (bone.node == 1)
				weight = weights[vi->Index()];

			pos += (skeleton->vert[bone.node] + bone.L * bw.lcoord) * weight;
		}
		if ((pos - targets[vi->Index()]).SquaredNorm() > 0.001)
			weights[vi->Index()] = -1;
	}

	for (CMesh::VertexIterator vi = meshes[0]->vert.begin(); vi != meshes[0]->vert.end(); vi++)
	{
		if (weights[vi->Index()] != -1)
		{
			for (int i = 0; i < vi->bone_weight.size(); i++)
			{
				if (vi->bone_weight[i].bone->node == 0)
					vi->bone_weight[i].weight = 1 - weights[vi->Index()];

				if (vi->bone_weight[i].bone->node == 1)
					vi->bone_weight[i].weight = weights[vi->Index()];
			}
		}
	}

	cout << GetRealTime() - t0 << endl;
#endif

	// return skeleton and mesh to rest position
	skeleton->RotateBone(1, math::ToRad(0.f), skeleton->dir_rot);
	//Update();	// update skeleton and meshes


	return err;

}

int CModel::GetNbMorphs()
{
	return meshes[0]->morphs.size();
}

void CModel::ApplyMorph(int index)
{
	if (GetNbMorphs() == 0)
		return;

	assert(index < GetNbMorphs());
	for (int i = 0; i < meshes.size(); i++)
		meshes[i]->ApplyMorph(index);
}

//  functor to compare two pairs <distance, *bone>
bool dist_bone_comp_func(pair<float, CBone*> i, pair<float, CBone*> j) 
{ 
	return (i.first < j.first); 
}
///////////////////////////// Performance Profiling \\\\\\\\\\\\\\\\\\\\\\\\\ 
void CModel :: ReadyUpade()
{
	skeleton->Update();
}
//3: MODEL:: update Skin for each frame  toooooooooooooooooooo much time */
// stiffnesstr_value, stiffnestet_value,stiffnesbone_value);
void CModel::Update(bool anim, int stiffstr)
	//void CModel::Update()
{ 
	//cout << anim;
	// update skeleton over the time 
	if (skeleton)
	{
		skeleton->Update();
		meshes[0]->skin.ske=skeleton;
		for ( int k =0; k <meshes[0]->skin._constraints.size(); k++)
			meshes[0]->skin._constraints[k].skel=skeleton;
	}// update vertice skin without animation
 /*   for (vector<Particle> ::iterator pi = meshes[0]->skin._particles.begin(); pi != meshes[0]->skin._particles.end(); pi++)
	{
       pi->Pstretch_stiffness=stiffstr/2;
	  pi->Pstet_stiffness=stifftet;
	  pi->Pbone_stiffness=stiffbone;
	}	*/
	meshes[0]->UpdateSkin(); // for the body just without the other meshes like eyes .... ect
	count++;
	//Saveobj();
	/*for (int vi =0; vi < meshes[0]->skin._particles.size(); vi++)
	{
	meshes[0]->skin._particles[vi].pos=meshes[0]->skin._particles[vi].pos;
	}*/
}
void CModel::Draw(bool error,bool show_Wire)
{
	//meshes[0]->DrawSkinError();
	//glEnable(GL_POLYGON_OFFSET_FILL);
	if(error)
		meshes[0]->DrawSkinError();
	// else if (skeleton &&  !error)
	// {
	if (1)
	meshes[0]->DrawSkin() ;// in order to draw teterhdral
	//meshes[0]->skin.DrawLinkBone();
	//cout << show_Wire;
	if(show_Wire)
		meshes[0]->DrawWireSkin();	
	//meshes[0]->DrawSkin() ;// in order to draw teterhdral
	//meshes[0]->DrawWireSkin();	
	//meshes[0]->DrawSkinError() ;// in order to draw teterhdral
	//skeleton->Draw();
	//  }
	if (skeleton)
		skeleton->Draw();
}

CModel::~CModel()
{
	for (int i = 0; i < meshes.size(); i++)
		delete meshes[i];
	meshes.clear();

	if (skeleton)
		delete skeleton;
}
// call linear blend skinning 
void CModel::updateLinearSkin(int stiffness,int strstiffness, int stiffnesstr_prevvalue)
{
	///// update the skin /////
	/// update the skin weight and make each vertex move according to
	/// skeleton 
	float stiff=stiffness;
	if (skeleton)
	{
		skeleton->Update();
		meshes[0]->skin.ske=skeleton;
		for ( int k =0; k <meshes[0]->skin._constraints.size(); k++)
			meshes[0]->skin._constraints[k].skel=skeleton;
	}
	// update vertice skin without animation
	for (int vi =0; vi < meshes[0]->skin._particles.size(); vi++)
	{
		Point3f pos(0, 0, 0);
		Point3f convert;
		//	if (meshes[0]->skin._particles[vi].isExternal) /*&& (p.isExternal)*/
		//	{
		vector<Particle::CPartWeight> ::iterator it= meshes[0]->skin._particles[vi].particle_weight.begin(); 
		for (; it != meshes[0]->skin._particles[vi].particle_weight.end(); it++)
		{

			Particle::CPartWeight & bw = *it;
			CBone & bone = *(bw.bone);
			convert.X() =bw.lcoord.x;
			convert.Y() =bw.lcoord.y;
			convert.Z() =bw.lcoord.z;
			/*cout << " world coordinate " <<bw.lcoord.x
			<< bw.lcoord.y
			<< bw.lcoord.z;*/
			if(strstiffness!=stiffnesstr_prevvalue)
			{
			double temp_stiff=0;
			temp_stiff=strstiffness/bw.distance;
			//cout << "\n distance:" <<bw.distance;
			temp_stiff=(temp_stiff* 1.0)/max_distance;
		     if(temp_stiff >1)
			meshes[0]->skin._particles[vi].Pstretch_stiffness=1;
			else
			meshes[0]->skin._particles[vi].Pstretch_stiffness=0.1+temp_stiff;
			//cout << "\n stretch: " << meshes[0]->skin._particles[vi].Pstretch_stiffness;
			}
			//cout << "\n stretch: " << meshes[0]->skin._particles[vi].Pstretch_stiffness;
	
	if (bw.distance<average_distance+3.8)

		pos += (skeleton->vert[bone.node] + bone.L * convert) * bw.weight;  // convert from bone coordinate to world
	else
	{
			pos.X()=meshes[0]->skin._particles[vi].pos.x;
			pos.Y()=meshes[0]->skin._particles[vi].pos.y;
			pos.Z()=meshes[0]->skin._particles[vi].pos.z;
		}
		}

		meshes[0]->skin._particles[vi].pos.x=pos.X();
		meshes[0]->skin._particles[vi].pos.y=pos.Y();
		meshes[0]->skin._particles[vi].pos.z=pos.Z();
		//}
	}
	//meshes[0]->skin.ComputeNormals();
}
// call smooth skinning 
void CModel::LinearSmoothSkin(CMesh *SmoothWeights)
{

	// for each particle v
	for (int i = 0; i < SmoothWeights->skin._particles.size(); i++)
	{
		Particle p=SmoothWeights->skin._particles[i];
		
		// init sum for each bone = weight of v for each bone
		vector<float> sum;
		sum.assign(skeleton->bone.size(), 0.f);
		for (int k = 0; k < p.particle_weight.size(); k++)
		{
			// sum for each bone = sum for each bone + weight of vi
			sum[p.particle_weight[k].bone->index] += p.particle_weight[k].weight;
		}
		//cout << " we inter here  star size" << p.neighs.size() ;
		std::set<int >::iterator it;
          for (it = p.neighs.begin(); it != p.neighs.end(); ++it)
           {
		    int n=*it;
		    Particle & np = *(SmoothWeights->skin.GetParticleAt(n));
           for (int k = 0; k < np.particle_weight.size(); k++)
			{
				// sum for each bone = sum for each bone + weight of vi
				sum[np.particle_weight[k].bone->index] += np.particle_weight[k].weight;
			}
		}

		// sum for each bone /= n_neighs + 1;
		for (int j = 0; j < sum.size(); j++)
			sum[j] = sum[j] / (float)p.neighs.size();

		for (int j = 0; j < sum.size(); j++)
			// for each entry of sum != 0
			if (sum[j] != 0)
			{
				//	if the corresponding bw !exists
				int l = -1;
				for (int k = 0; k < p.particle_weight.size(); k++)
				{
					if (p.particle_weight[k].bone->index == j)
					{
						l = k;
						break;
					}
				}

				if (l == -1)
				{
					//		create bw
					Particle::CPartWeight bw;
					bw.bone = &(skeleton->bone[j]);
					bw.weight = sum[j];

					p.PushPartBoneWeight(bw);
				}
				else
				{
					//	update bw.weight = entry 
					p.particle_weight[l].weight = sum[j];
				}
			}
	
	}
	FinalizeWeights(SmoothWeights);
}
// call linear blend skinning 
void CModel::LinearSkin(CMesh *SmoothWeights)
{
	average_distance=0;
	//double t0 = GetRealTime();
	//cout << "particle nearest bone assginment ... \n";
	//	meshes[0]=SmoothWeights;
	int n_bones =1;
	// for each particle v
	for (int i = 0; i <SmoothWeights->skin._particles.size(); i++)
	{	
		vector<pair<float, CBone*>> distances;
		Particle & p =SmoothWeights->skin._particles[i];
		if (p.isExternal)
		{
		//for each bone b
		vector<CBone>::iterator bi = skeleton->bone.begin();
		for (; bi != skeleton->bone.end(); bi++)
		{
			CBone & bone = *bi;	
			// get the corresponding segment {vert here we are getting bone in Skeleton}
			Point3f & s = skeleton->vert[bone.node];      // node[joint] O=========O child[joint] // actual
			Point3f & d = skeleton->vert[bone.child];     //////////////   bone  ///////////////
			Segment3f seg_bone(s, d); // our segment bone
			Point3f closest;
			Point3f par;
			par.X()=p.pos.x;
			par.Y()=p.pos.y;
			par.Z()=p.pos.z;
			float dist;
			SegmentPointDistance<float>(seg_bone, par, closest, dist);
	          distances.push_back(pair<float, CBone*>(dist, &bone)); // keep distance and bone

			if ((d-closest)<(d-seg_bone.MidPoint())|| (s-closest)<(s-seg_bone.MidPoint()))
				n_bones=1;
		//	if ( skeleton->vert[bone.child] ==NULL)
             //       n_bones=1;

		}
	
		sort<vector<pair<float, CBone*>>::iterator>(	distances.begin(),  
			distances.end(), 
			dist_bone_comp_func_skeP);  // sort all bone based on there distance from vertex

		max_distance=distances[distances.size()-1].first;
		// get the n bones with smallest distance
		for (int j = 0; j < n_bones; j++)
		{      
			   if (distances[j].first<1.3)
			   {
			assert(distances.size() >= n_bones);
			int vertexindex = p.index;
			int boneindex= 0.1f/distances[j].second->index;
			int boneindexchild= distances[j].second->child;
			//////////// assign nearest bone to vertex /////////////
			parassignbone.push_back(make_pair(vertexindex,boneindex));
			//////////////////// *************assgining weight *************** /////////////////
			Particle::CPartWeight bw;
			bw.bone = distances[j].second; // assgin for each particle nearest bone
			//cout<< " bw distance" << bw.distance <<endl; 
			bw.weight = 1.0f/distances[j].first; // inital or reset distance with neatest bone
			p.PushPartBoneWeight(bw); ////// add weight
			//p.oldPos=p.pos;
		}
		}
	}
	}
	FinalizeWeights(SmoothWeights);
	averageDistance(SmoothWeights);
	cout << " the distance to the bone " << average_distance;
}
void CModel::FinalizeWeights(CMesh *SmoothWeights)
{
	//Normalize weights
	//cout << " inside find AssginParticlesWeights------------------? " << mesh->skin._particles.size()  ; 
	for (vector<Particle>::iterator pi = SmoothWeights->skin._particles.begin(); pi != SmoothWeights->skin._particles.end(); pi++)
		pi->NormalizeParWeights();

	//Assign coordinates in the local frame of the bone to which the vertex belongs to

	Update(1,1);
	for (vector<Particle> ::iterator pi = SmoothWeights->skin._particles.begin(); pi != SmoothWeights->skin._particles.end(); pi++)
	{
		for (vector<Particle::CPartWeight> ::iterator it= SmoothWeights->skin._particles[pi->index].particle_weight.begin(); 
			it !=SmoothWeights->skin._particles[pi->index].particle_weight.end(); it++)
		{

			//Particle::particle_weight.
			Particle::CPartWeight & bw = *it;
			CBone & bone = *(bw.bone);
			Point3f vpos;
			vpos.X()=pi->pos.x;
			vpos.Y()=pi->pos.y;
			vpos.Z()=pi->pos.z;
			Point3f v = vpos - skeleton->vert[bone.node];
			float lx = v * bone.x;
			float ly = v * bone.y;
			float lz = v * bone.z;
			bw.lcoord = Vector3(lx, ly, lz); // from local coordinate
			//double temp_stiff=0;
			//temp_stiff=1/bw.distance;
		//	cout << "\n distance:" <<bw.distance;
			//temp_stiff=(temp_stiff* 1.0)/max_distance*10;
			//if(temp_stiff >1)
			//pi->Pstretch_stiffness=1;
			//else
			//pi->Pstretch_stiffness=temp_stiff;
			//cout << "\n stretch:" << pi->Pstretch_stiffness;
			
		}
	}		
}
//
///////////////////////////////// write every thing inside obj file ///////////////////
void CModel :: Saveobj()
{
	// open and write data to the file
	std::string file_name =bvhname ;
	ostringstream ss;
	ss << count;
	file_name= file_name.append(ss.str());
	file_name= file_name.append(".obj");
	const char *sptr = file_name.c_str() ;
	//qWarning() << " IF You Get My NAME "<<  sptr;
	/* open the file */
	FILE* objfile; 
	objfile = fopen(sptr, "w");
	if (!objfile) {
		fprintf(stderr, "WriteOBJ() failed: can't open file \"%s\" to write.\n",
			objfile);
		exit(1);
	}
	/* spit out a header */
	fprintf(objfile,"#  \n");
	fprintf(objfile,"#  Wavefront OBJ \n");
	fprintf(objfile,"#  \n");
	fprintf(objfile,"#  Nadine ^_____^ \n");
	fprintf(objfile,"#  \n");
	/* spit out the vertices */
	fprintf(objfile,"# %d vertices\n",meshes[0]->skin._particles.size());
	// fprintf(objfile,"%d %d %d",meshes[0]->skin._particles.size(),meshes[0]->skin.nb_tri , 0);
	for (int i = 0; i <meshes[0]->skin._particles.size(); i++)
	{
		fprintf(objfile, "v %f %f %f\n", 
			meshes[0]->skin._particles[i].pos.x,
			meshes[0]->skin._particles[i].pos.y,
			meshes[0]->skin._particles[i].pos.z);
	}
	///   std::sort (meshes[0]->skin._faces.begin(), meshes[0]->skin._faces.end());  
	// fprintf(objfile, "# %d Face \n\n",meshes[0]->skin.nb_tri);
	for (int i = 0; i <meshes[0]->skin.nb_tri; i++)
	{
		int nV = i*3;
		fprintf(objfile, "f %d %d %d\n", 
			meshes[0]->skin._faces[nV]+1,
			meshes[0]->skin._faces[nV+1]+1,
			meshes[0]->skin._faces[nV+2]+1);
	}
	fclose(objfile);
}
void CModel :: averageDistance(CMesh *SmoothWeights)
{

	int count =0;
	for (vector<Particle> ::iterator pi = SmoothWeights->skin._particles.begin(); pi != SmoothWeights->skin._particles.end(); pi++)
	{
	 for (vector<Particle::CPartWeight> ::iterator it= SmoothWeights->skin._particles[pi->index].particle_weight.begin(); 
		it !=SmoothWeights->skin._particles[pi->index].particle_weight.end(); it++)
		
	      {
               count ++;
			Particle::CPartWeight & bw = *it;
		      total_distance+= it->distance;
          }
	}
	average_distance = total_distance/ count;

}