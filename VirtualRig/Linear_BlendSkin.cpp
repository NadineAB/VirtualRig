#  pragma warning(disable:4002)
#include "skeleton.h"
#include "Linear_BlendSkin.h"
#include "timer.h"
//#define USE_PROFILE		// undef to disable profiling
#include "profile.h"
Linear_BlendSkin::Linear_BlendSkin()
{
}
void Linear_BlendSkin::FinalizeWeights(CMesh *SmoothWeights)
{
/*	//Normalize weights
     for (vector<Particle>::iterator pi = SmoothWeights->skin._particles.begin(); pi != SmoothWeights->skin._particles.end(); pi++)
	pi->NormalizeParWeights();
	
	//Assign coordinates in the local frame of the bone to which the vertex belongs to
	Update(1);
	for (vector<Particle> ::iterator pi = SmoothWeights->skin._particles.begin(); pi != SmoothWeights->skin._particles.end(); pi++)
	{
		for (vector<Particle::CPartWeight> ::iterator it= SmoothWeights->skin._particles[pi->index]..begin(); 
			it !=SmoothWeights->skin._particles[pi->index].particle_weight.end(); it++)
	{

			//Particle::particle_weight.
			Particle::CPartWeight & bw = *it;
			CBone & bone = *(bw.bone);
			Point3f vpos;
			vpos.X()=pi->pos.x;
			vpos.Y()=pi->pos.y;
			vpos.Z()=pi->pos.z;
			Point3f v = vpos - Linearskeleton->vert[bone.node];
			float lx = v * bone.x;
			float ly = v * bone.y;
			float lz = v * bone.z;
			bw.lcoord = Vector3(lx, ly, lz); // from local coordinate
		}
	}
	*/
}
///////////////////////////// Performance Profiling \\\\\\\\\\\\\\\\\\\\\\\\\ 
void Linear_BlendSkin :: ReadyUpadteSkel()
{
	Linearskeleton->Update();
}
//3: MODEL:: update Skin for each frame too much time */
void Linear_BlendSkin::LBSUpdate()
{
	if (Linearskeleton)
		Linearskeleton->Update();

	///// update the skin /////
	/// update the skin weight and make each vertex move according to
	/// skeleton 
	for (int vi =0; vi < LinerSkinmeshes[0]->vert.size(); vi++)
	{
		Point3f pos(0, 0, 0);
		Point3f convert;

		vector<Particle::CPartWeight> ::iterator it= LinerSkinmeshes[0]->skin._particles[vi].particle_weight.begin(); 
		for (; it != LinerSkinmeshes[0]->skin._particles[vi].particle_weight.end(); it++)
		{
			Particle::CPartWeight & bw = *it;
			CBone & bone = *(bw.bone);
			convert.X() =bw.lcoord.x;
			convert.Y() =bw.lcoord.y;
			convert.Z() =bw.lcoord.z;
			/*cout << " world coordinate " <<bw.lcoord.x
			<< bw.lcoord.y
			<< bw.lcoord.z;*/
			pos += (Linearskeleton->vert[bone.node] + bone.L * convert) * bw.weight;  // convert from bone coordinate to world
		}
		LinerSkinmeshes[0]->vert[vi].P().X()=pos.X();
		LinerSkinmeshes[0]->vert[vi].P().Y()=pos.Y();
		LinerSkinmeshes[0]->vert[vi].P().Z()=pos.Z();
	}
}
Linear_BlendSkin::~Linear_BlendSkin()
{
	for (int i = 0; i < LinerSkinmeshes.size(); i++)
		delete LinerSkinmeshes[i];
	LinerSkinmeshes.clear();

	if (Linearskeleton)
		delete Linearskeleton;
}
