#include "skeleton.h"
#include <vector>
#include <iostream>
#include <assert.h>
#include "particle.h"
#include "constraint.h"
//#define USE_PROFILE		// endef to disable profiling
#include "profile.h"
using namespace std;
// empty constructor
#define INITIAL_TRIANGLE 10000000
Constraint :: Constraint()
{
	ref_parts.clear();
	ref_bones.clear();
	type = CONSTR_NONE;
	rest_value = 0.f;
	stiffness = 1.f;
	phase = -1;
//	ref_faces = new Bag<Triangle*>(INITIAL_TRIANGLE,Triangle::extract_func);

}
// constructor to set the default value
Constraint:: Constraint(ConstraintType _type, float _rest_value,float _stiffness)
{
	rest_value	= _rest_value;
	stiffness	= _stiffness;
	type		= _type;
}

void Constraint::SetRestVolume()
{
	float _volume = -1;
	if ((type != VOLUME) && (type != TETRA_VOLUME))
	{
		printf("Constraint::SetRestVolume: ehy, I am not a volume constraint!\n");
		return;
	}
	if (_volume < 0)
	{
		if (type == VOLUME)
			rest_value = ComputeVolume();
		if (type == TETRA_VOLUME)
			rest_value = ComputeVolumeTetra();
	}
	else
		rest_value = _volume;
}
void Constraint::Solve(SolverType solver_type)
{   
	PROFILE_SAMPLE("SolveConstraint");
	switch (type)
	{
	case STRETCH:
		SolveStretch(solver_type);
		break;
	case BENDING:
		SolveBending(solver_type);
		break;
	case BONESTRETCH:
		SolveStretchBone(solver_type);
		break;
	case AREA:
		SolveTriArea(solver_type);
		break;
	case VOLUME:
		SolveVolume(solver_type);
		break;
	case TETRA_VOLUME:
		SolveVolumeTetra(solver_type);
		break;
	}
}
// distance between p0 and p1 must be rest_value
void Constraint ::SolveStretchBone(SolverType & solver_type)
{
	// what I send it to here is the rest postion //
	Point3f vpos,cnearestpoint, new_vector;
	float current_distance,new_distance;
	PROFILE_SAMPLE("SolveStretch");
	Particle & p0 = *ref_parts[0]; // the old vertices postion
	Particle::CPartWeight & nb = * ref_bones[0]; // the nearest bone 
	// we need this new postion of the bone 
	Vector3 ndelta,odelta,temp, update_pos;
	vpos.X()=p0.pos.x; // particle postion
	vpos.Y()=p0.pos.y;
	vpos.Z()=p0.pos.z; 
	// have the segment of bone
	Segment3f seg_bone(skel->vert[nb.bone->node], skel->vert[nb.bone->child]); // segment bone
	SegmentPointDistance<float>(seg_bone, vpos, cnearestpoint, new_distance);		
	current_distance=new_distance; // current lenght
	float diff = new_distance- rest_value; // difference between the new distance and reset value
	// compute the new vector from the point to nearest point
	new_vector = cnearestpoint -vpos ;
	new_vector = new_vector.normalized(); // normalize the vector, so it will be direction
	update_pos.x=new_vector.X();
	update_pos.y=new_vector.Y();
	update_pos.z=new_vector.Z();
	Vector3 newjointpos,strech;
	newjointpos.x= skel->vert[nb.bone->node].X();
	newjointpos.y= skel->vert[nb.bone->node].Y(); 
	newjointpos.z= skel->vert[nb.bone->node].Z(); 
	// old postion
	odelta= nb.mdelta;
	// new postion of the bones 
	temp.x=cnearestpoint.X();
	temp.y=cnearestpoint.Y();
	temp.z=cnearestpoint.Z();
	//cout << " local coordinate " << convert.X() << convert.Y() << convert.Z();
	ndelta =temp-p0.pos;
	float deltaLength=(ndelta-odelta).Length();
	float mintian = (temp-odelta).Length();
	/*
	based on the parmteric line equation */
	// find delta v
	Point3f nppos,deltav,nvpos;
	//cout << "\n \n tp  ---------->" << nb.tp  ;
	nppos =  skel->vert[nb.bone->node] *nb.tp + skel->vert[nb.bone->child] *( 1-nb.tp);
	//cout << "\n here the new point in the bone "<< nvpos.X() << nvpos.Y() << nvpos.Z();
	deltav=(vpos-nppos).Normalize() * ((vpos-nppos).Norm() - rest_value);
	// the new postion of the particle 
	nvpos = vpos - deltav;
	//nb.bone->
	if (solver_type == GAUSS_SEIDEL_RELAX)
	{
		if(diff==0)
			p0.pos =p0.pos;   // do nothing if the joint not moving
		else 
		{
		//	cout << p0.pos.x;
			p0.pos.x=  nvpos.X();
			p0.pos.y=  nvpos.Y();
			p0.pos.z=  nvpos.Z();
		     //cout <<nb.lcoord.x << nb.lcoord.y << nb.lcoord.z; 
			//p0.pos  = p0.pos + update_pos*diff*1; //// Working one

             //p0.pos  = newjointpos + update_pos*diff*1; //// here
			/*Point3f point;
			convert.X() =nb.lcoord.x;
			convert.Y() =nb.lcoord.y;
			convert.Z() =nb.lcoord.z;
			Particle::CPartWeight & bw = nb;
			CBone & bone = *(bw.bone);
			point =(skel->vert[nb.bone->node] + bone.L* convert);
			p0.pos.x  = p0.pos.x + point.X();
			p0.pos.y  = p0.pos.x + point.Y();
			p0.pos.z  = p0.pos.x + point.Z();
			//update_pos  =update_pos*diff;
			/*p0.pos.x = temp.x ;
			p0.pos.y = temp.y ;
			p0.pos.z = temp.z ;*/
			/*+ 
			//nb.bone->L.operator*(
			//* nb.lcoord ;
			/*p0.pos=temp-odelta;
			p0.pos.x= nb.bone->L.operator*(vpos).X();
			p0.pos.y= nb.bone->L.operator*(vpos).Y();
			p0.pos.z= nb.bone->L.operator*(vpos).Z();*/
		}
		//(temp-odelta);
	}  
	Vector3 newdiff;
	newdiff.x=cnearestpoint.X()-p0.pos.x;
	newdiff.y=cnearestpoint.Y()-p0.pos.y;
	newdiff.z=cnearestpoint.Z()-p0.pos.z;
	double error=0.0;
	error=newdiff.Length()-rest_value;
	p0.errorpos=diff;
	//p0.errorpos=error;

}
void Constraint ::SolveStretch(SolverType & solver_type)
{
	PROFILE_SAMPLE("SolveStretch");
	assert(ref_parts.size() == (size_t)2);
	Particle & p0 = *ref_parts[0];
	Particle & p1 = *ref_parts[1];
	Vector3 delta = p1.pos - p0.pos;
	float push=0.1;
	float pull=0.5;
	float deltaLength = delta.Length();
	float diff = (deltaLength - rest_value) / deltaLength;
	if (!p0.isFixed)
	{
		if (solver_type == GAUSS_SEIDEL_RELAX)
			p0.pos += delta * 0.5 * diff *  p0.Pstretch_stiffness;
		if (solver_type == JACOBI)
			p0.delta_p += delta * 0.125 * diff * p0.Pstretch_stiffness;
	}
	if (diff==0)
	{
	}
	else
	{
	if (!p1.isFixed)
		if (solver_type == GAUSS_SEIDEL_RELAX)
			p1.pos -= delta * 0.5 * diff *  p1.Pstretch_stiffness;
	if (solver_type == JACOBI)
		p1.delta_p -= delta * 0.125 * diff * p1.Pstretch_stiffness;
	}
}
// angle between two triangular faces (p0,p1,p2) and (p0,p3,p1) must be rest_value
void Constraint:: SolveBending(SolverType & solver_type)
{
	PROFILE_SAMPLE("SolveBending");
	assert(ref_parts.size() == (size_t)4);
	Particle & p0 = *ref_parts[0];   /*        * p2		*/
	Particle & p1 = *ref_parts[1];	 /*       / \		*/
	Particle & p2 = *ref_parts[2];	 /*      /   \		*/
	Particle & p3 = *ref_parts[3];	 /*	 p0 *-----* p1	*/
	/*      \   /		*/
	/*       \ /		*/
	/*        * p3		*/
	// translate everything in p0
	Vector3 _p0 = p0.pos;
	Vector3 _p1 = p1.pos - _p0;
	Vector3 _p2 = p2.pos - _p0;
	Vector3 _p3 = p3.pos - _p0;
	Vector3 n0 = (_p1 ^ _p2).Normalize();
	Vector3 n1 = (_p1 ^ _p3).Normalize();;
	float d = n0 * n1;
	if (d > 1) d = 1;	// prevent numerical drift
	if (d < -1) d = -1;
	Vector3 q[4];
	q[3] = ((_p1 ^ n0) + (n1 ^ _p1) * d) / (_p1 ^ _p3).Length();
	q[2] = ((_p1 ^ n1) + (n0 ^ _p1) * d) / (_p1 ^ _p2).Length();
	q[1] = ((((_p2 ^ n1) + (n0 ^ _p2) * d) / (_p1 ^ _p2).Length()) +
		(((_p3 ^ n0) + (n1 ^ _p3) * d) / (_p1 ^ _p3).Length())) * -1;
	q[0] = (q[1] + q[2] + q[3]) * -1;
	float sum_q = 0;
	for (int i = 0; i < 4; i++)
		sum_q += q[i].SquaredLength();
	if (sum_q < 0.00001) return;		// avoid overflow and division by zero
	// so far, all the masses are considerer like 1
	float s = -stiffness * (sqrt(1 - d * d) * (acosf(d) - rest_value)) / sum_q;
	float diff = acosf(d) - rest_value;
	for (int i = 0; i < 4; i++)
		if (!ref_parts[i]->isFixed)
		{
			ref_parts[i]->pos += q[i] * s;
		}
		//printf(" here slove for bending ");
}
// area of the triangular face (p0,p1,p2) must be rest_value
void Constraint ::SolveTriArea(SolverType & solver_type)
{
	assert(ref_parts.size() == (size_t)3);
	Particle * p[3];
	p[0] = ref_parts[0];
	p[1] = ref_parts[1];
	p[2] = ref_parts[2];
	float area = ((p[1]->pos - p[0]->pos) ^ (p[2]->pos - p[0]->pos)).Length() / 2.f;
	float c = area - rest_value;
	if (fabs(c) < 0.00001f)
		return;
	Vector3 grad[3];
	for (int i = 0; i < 3; i++)
	{
		Vector3 e2 = (p[(i + 1) % 3]->pos - p[i]->pos);
		Vector3 e1 = (p[(i + 2) % 3]->pos - p[i]->pos);
		Vector3 e0 = (p[(i + 2) % 3]->pos - p[(i + 1) % 3]->pos);
		Vector3 n = e1 ^ e2;
		Vector3 h = e0 ^ n;
		grad[i] = h.Normalize() * e0.Length() * .5;
	}
	float s = c / (grad[0] * grad[0] + grad[1] * grad[1] + grad[2] * grad[2]);
	for (int i = 0; i < 3; i++)
		if (!p[i]->isFixed)
			p[i]->pos += grad[i] * -s; 
}
// it constraints the particles to move in order to fit the desired volume rest_value
void Constraint ::SolveVolume(SolverType & solver_type)
{
	float C = rest_value - ComputeVolume();

		if (fabs(C) < 0.00001f)
			return;

		float sum_squared_grad_p = 0;	// denominator of the scaling factor
		float s = 0;										// scaling factor

		// for each particle, compute the corresponding gradient
		vector<Particle *>::iterator pit = ref_parts.begin();
		for (; pit != ref_parts.end(); pit++)
		{
			Particle & pa = **pit;
			pa.grad = Vector3(0, 0, 0);

			set<int>::iterator fit = pa.faces.begin();
			for (; fit != pa.faces.end(); fit++)
			{
				int j = 0;
				for (; j < 3; j++)
				{
					int iPart = (ref_faces[*fit])[j];
					if (iPart == pa.index)
						break;
				}

				assert(j != 3);

				// debug
				//int i0 = (ref_faces[*fit])[j];
				//int i1 = (ref_faces[*fit])[(j + 1) % 3];
				//int i2 = (ref_faces[*fit])[(j + 2) % 3];

				Vector3 vb =  ref_parts[(ref_faces[*fit])[(j + 1) % 3]]->pos;
				Vector3 vc =  ref_parts[(ref_faces[*fit])[(j + 2) % 3]]->pos;

				pa.grad += vb ^ vc;
//				printf("[%d]: %d %d %d %.4f\t", pa.index, i0, i1, i2, (vb ^ vc).Length());
			}
//			printf("[%d]: %.4f\t", pa.index, pa.grad);
		}


		for (pit = ref_parts.begin(); pit != ref_parts.end(); pit++)
			sum_squared_grad_p += (**pit).grad.SquaredLength();

//		printf("\n sum sq grads: %.4f\t", sum_squared_grad_p);

		if (sum_squared_grad_p > 0)
			s = C / sum_squared_grad_p;
		else
			s = 0;

		// move the particles in order to fill the target volume
		for (pit = ref_parts.begin(); pit != ref_parts.end(); pit++)
		{
			Particle & p = **pit;
			if (!p.isFixed)
				p.pos += p.grad * s * stiffness;
		}

//		printf("s: %.4f\n", s);

}
void Constraint ::SolveVolumeTetra(SolverType & solver_type)
{
	assert(ref_parts.size() == (size_t)4);
	// compute the differance in volume between the volume and rest volume
	double C = rest_value - ComputeVolumeTetra();
    // cout << "\n reset pose " << rest_value;
    //cout << "\n volume " << ComputeVolumeTetra();
	
	////////////////////// get the particles for tetrahderal ///////////
	Particle & p0 = *ref_parts[0];   
	Particle & p1 = *ref_parts[1];	 
	Particle & p2 = *ref_parts[2];	 
	Particle & p3 = *ref_parts[3];	 
	// work based on translate the particle to p0
	// translate everything in p0
	Vector3 _p0 = p0.pos;
	Vector3 _p1 = p1.pos - _p0;
	Vector3 _p2 = p2.pos - _p0;
	Vector3 _p3 = p3.pos - _p0;

	Vector3 grad[4];	
	// gradients computed wrt each vertex of the tetrahedra
	grad[1] = (_p2 ^ _p3) * (1.f/6.f);
	grad[2] = (_p3 ^ _p1) * (1.f/6.f);
	grad[3] = (_p1 ^ _p2) * (1.f/6.f);

	grad[0] = (grad[1] + grad[2] + grad[3]) * -1.f;

	//for (int i = 0; i < 4; i++)
	//	if (grad[i].SquaredLength() > 10.f)
	//		return;

	float sum_squared_grad_p = 0;	// denominator of the scaling factor
	for (int i = 0; i < 4; i++)
		sum_squared_grad_p= grad[i] * grad[i];
	// scaling factor based of for 4 tetera
	double s = 0;
	if (sum_squared_grad_p > 0.1f)
		s = C / sum_squared_grad_p; // the differance divide by sum of grid
	//else if(sum_squared_grad_p > 0.001f &&  sum_squared_grad_p > 0.0001f)
		//s = 0;
	else 
           s = 0;

	// move the particles in order to fill the target volume
	for (int i = 0; i < 4; i++)
	{    /// stiff one
		Particle & p = *ref_parts[i];
		Vector3 & v0 = p.pos;
		Vector3 delta = grad[i] * s *1.0;
		// get the delta differance between the volume 
		//9
		if ((!p.isFixed) /*&& (p.isExternal)*/ && (delta.Length() <3/*fabs(rest_value)*/) )
		{
			if (solver_type == GAUSS_SEIDEL_RELAX)
				p.pos += delta * (1/36.f)/*grad[i] * s * stiffness * (1/36.f)*/;
			if (solver_type == JACOBI)
				p.delta_p += delta * (1/36.f);
		}
		p0.errorvol=C;
	}
}
// (quite) fast volume computation 
// the volume is embedded by the triangulat faces stored in ref_faces
float Constraint ::ComputeVolume()
{
	float V = 0;
	int i=0;
	for ( int nTri = 0; nTri < ref_faces.size()/3; nTri++ )  // for loop the number of triangles 
	{
		int nV = nTri*3;
		Vector3 v0  = ref_parts[ *ref_faces[ nV ] ]->pos;
		Vector3 v1  = ref_parts[ *ref_faces[ nV+1 ] ]->pos;
		Vector3 v2  = ref_parts[ *ref_faces[ nV+2 ] ]->pos;	
		//cout<<  v0.x << " " << v0.y <<" "<< v0.z << endl;
		//cout<<  v1.x << " " << v1.y <<" "<< v1.z << endl;
		//cout<<  v2.x << " " << v2.y <<" "<< v2.z << endl;
		V += (v0 ^ v1) * v2;
		//cout << " vloumeeeeeeeeeeeee" << abs(V)<< endl;
	}
	return abs(V);// / 6.f;
}
// compute the volume of the tetra element
float Constraint ::ComputeVolumeTetra()
{
	// 1/6 (crossproduct and dotproduct)
	assert(ref_parts.size () == 4);
	float V = 0;
	Vector3 p0 = ref_parts[0]->pos;
	Vector3 p10 = ref_parts[1]->pos - p0;
	Vector3 p20 = ref_parts[2]->pos - p0;
	Vector3 p30 = ref_parts[3]->pos - p0;
	V = ((p10 ^ p20)* p30) / 6.f;
	//cout << " volume ----------------> " << V << endl;
	return V;
}