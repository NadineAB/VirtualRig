#include "skeleton.h"
#include "mesh.h"

using namespace boost;

int CSkeleton::Load()
{
	vert.clear();
	bone.clear();
	mesh = NULL;

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

			CBone b;
			Point3f p;
			b.node = data[0];
			p = Point3f(data[1], data[2], data[3]);
			b.parent = data[4];
			bone.push_back(b);
			vert.push_back(p);
		}
	}	
	while (!stream.eof());  

	Init();

	return result;
}

void CSkeleton::Reset()
{
	for (int i = 0; i < bone.size(); i++)
	{
		vert[i] = bone[i].rpos;
//		bone[i].prevpos = bone[i].rpos;

		bone[i].L.SetIdentity();
	}
	
	UpdateLocalFrame();
}

void CSkeleton::Init()
{
	// assign prev and rest pos
	for (int i = 0; i < bone.size(); i++)
	{
		bone[i].prevpos = vert[i];
		bone[i].rpos = vert[i];
	}

	// assign childs
	for (int i = 0; i < bone.size(); i++)
	{
		if (bone[i].parent == -1)	// skip the root
			continue;

		CBone & parent = bone[bone[i].parent];
		parent.child.push_back(bone[i].node);	// note: bone[i].node is equal to i
	}

	for (int i = 0; i < bone.size(); i++)
	{
		if (bone[i].parent == -1)	// this is the root
			continue;

		Point3f & s = vert[bone[i].node];
		Point3f & d = vert[bone[i].parent];

		float rdist = (s - d).Norm();
		bone[i].rdist = rdist;			// init rest distance
	}

	for (int i = 0; i < bone.size(); i++)
	{
		// init local reference frame
		bone[i].L.SetIdentity();

		Point3f & s = vert[bone[i].node];
		bbox.Add(s);					// init bounding box
	}

	// assign each vertex of the mesh to a local reference frame of the matrix
	if (mesh)
	for (CMesh::VertexIterator vi = mesh->vert.begin(); vi != mesh->vert.end(); vi++)
	{
		Point3f & vpos = vi->P();
		float _min = std::numeric_limits<float>::max();
		int index = -1;

		for (int i = 0; i < bone.size(); i++)
		{
			if (bone[i].child.empty())
				continue;
			// find the bone nearest to the current vertex
			Point3f bpos = vert[i];
			Point3f cpos = vert[bone[i].child[0]];
			Segment3f seg_bone(bpos, cpos);
			float dist = (vpos - bpos).Norm();

			Point3f clos;
			SegmentPointSquaredDistance<float>( seg_bone, vpos, clos, dist); 
			if (dist < _min)
			{
				_min = dist;
				index = i;
			}
		}

		bone[index].vp.push_back(&*vi);
	}

}

void CSkeleton::Assign(CMesh * _mesh)
{
	mesh = _mesh ;
}

void CSkeleton::UpdateLocalFrame()
{
	for (int i = 0; i < bone.size(); i++)
	{
		if (bone[i].child.empty())
			continue;

		// get current direction
		Point3f & curr_s = vert[bone[i].node];
		Point3f & curr_d = vert[bone[i].child[0]];

		Point3f curr_dir = (curr_d - curr_s).Normalize();
		
		// get previous direction
		Point3f & prev_s = bone[i].prevpos;
		Point3f & prev_d = bone[bone[i].child[0]].prevpos;

		Point3f prev_dir = (prev_d - prev_s).Normalize();

		// compute the rotation matrix
		Matrix33<float> mrot = RotationMatrix(prev_dir, curr_dir);

		// apply rotation matrix to local frame
		bone[i].L = mrot /** bone[i].L*/;

		for (int j = 0; j < bone[i].vp.size(); j++)
		{
			bone[i].vp[j]->P() -= curr_s;
			bone[i].vp[j]->P() = bone[i].L * bone[i].vp[j]->P();
			bone[i].vp[j]->P() += curr_s;
		}
	}
	for (int i = 0; i < bone.size(); i++)
	{
		bone[i].prevpos = vert[i];
	}
#if 0
	// compute z axis of the local coordinate frame
	for (int i = 0; i < bone.size(); i++)
	{
		bone[i].z = Point3f(0, 0, 0);
		Point3f & node = vert[bone[i].node];
		for (int j = 0; j < bone[i].child.size(); j++)
		{
			Point3f child = vert[bone[i].child[j]];

			Point3f z = child - node;
			z.Normalize();

			bone[i].z += z;
		}
		bone[i].z.Normalize();
	}

	// compute x axis of the local coordinate frame
	for (int i = 0; i < bone.size(); i++)
	{
		bone[i].x = Point3f(0, 0, 0);
		Point3f & node = vert[bone[i].node];
		if (bone[i].parent == -1)	// skip root by now
			continue;

		Point3f & parent = vert[bone[i].parent];
		Point3f p = parent - node;
		p.Normalize();

		bone[i].x = p ^ bone[i].z;
		bone[i].x.Normalize();
	}

	// compute y axis of the local coordinate frame
	for (int i = 0; i < bone.size(); i++)
	{
		bone[i].y = Point3f(0, 0, 0);
		Point3f & node = vert[bone[i].node];
		if (bone[i].parent == -1)	// skip root by now
			continue;

		bone[i].y = bone[i].x ^ bone[i].z;
		bone[i].y.Normalize();
	}
#endif
}

void CSkeleton::Update()
{

	int nbIters = 10;
	for (int i = 0; i < nbIters; i++)
	{
		for (int i = 0; i < bone.size(); i++)
		{
			if (bone[i].parent == -1)	// this is the root
				continue;
			assert(bone[i].node == i);
			Point3f & s = vert[bone[i].node];
			Point3f & d = vert[bone[i].parent];

			Point3f delta = s - d;
			float dist = delta.Norm();

			double diff = (dist - bone[i].rdist) / dist;

			s -= delta * 0.5 * diff * 1.f;//stiffness;
			d += delta * 0.5 * diff * 1.f;//stiffness;
		}
	}

	UpdateLocalFrame();

	// update prev pos
	for (int i = 0; i < bone.size(); i++)
		bone[i].prevpos = vert[i];
}

void CSkeleton::Draw()
{
	glDisable(GL_LIGHTING);
	glEnable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	glBegin(GL_LINES);
	for (int i = 0; i < bone.size(); i++)
	{
		if (bone[i].parent == -1)	// this is the root
			continue;
		assert(bone[i].node == i);
		Point3f & s = vert[i];
		Point3f & d = vert[bone[i].parent];
		glColor4f(1, 1, 1, 1);	// joint
		glVertex(s);
		glVertex(d);
		glColor4f(0, 0, 1, 1);	// z axis
		glVertex(s);
		glVertex(s + bone[i].Z() * bbox.Diag() * 0.05);
		glColor4f(1, 0, 0, 1);	// x axis
		glVertex(s);
		glVertex(s + bone[i].X() * bbox.Diag() * 0.05);
		glColor4f(0, 1, 0, 1);	// y axis
		glVertex(s);
		glVertex(s + bone[i].Y() * bbox.Diag() * 0.05);
	}
	glEnd();
	glPointSize(5.f);
	glBegin(GL_POINTS);
	glColor4f(0, 1, 1, 1);
	Point3f & s = vert[0];
	glVertex(s);
	glColor4f(1, 0, 0, 1);
	for (int i = 1; i < bone.size(); i++)
	{
		assert(bone[i].node == i);
		Point3f & s = vert[i];
		glVertex(s);
	}
	glEnd();
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
}
