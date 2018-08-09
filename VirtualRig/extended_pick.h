
#ifndef EXTENDED_PICK_H
#define EXTENDED_PICK_H
#include <wrap/gl/pick.h>
#include <vcg/complex/algorithms/closest.h>
#include <vcg/space/ray3.h>
#include <vcg/space/triangle3.h>
namespace vcg {
	/*! 
	* \brief:
	*        Base class used to provide user ability to pick vertices and
	*        pick joint according to mouse postion
	*        where you can pick vertices or faces
	* Coded by Marco Fratarcangeli
	*/
	template <class MESH_TYPE, class GRID>
	class GLExtendedPickTri : public GLPickTri<MESH_TYPE>
	{
	public:

		typedef typename MESH_TYPE::FacePointer	FacePointer;
		typedef typename MESH_TYPE::CoordType	CoordType;
		typedef typename MESH_TYPE::ScalarType	ScalarType;

#ifdef DEBUG_PICK
		static vector<CoordType> origs;
		static vector<CoordType> dests;
		static vector<CoordType> inters;
#endif

		static unsigned int PickFaces(int x, int y, MESH_TYPE &m, GRID &g, vector<FacePointer> & face)
		{
			// draw a ray from the mouse position
			double res[3];
			GLdouble mm[16],pm[16]; GLint vp[4];
			glGetDoublev(GL_MODELVIEW_MATRIX, mm);
			glGetDoublev(GL_PROJECTION_MATRIX, pm);
			glGetIntegerv(GL_VIEWPORT, vp);

			// project 2D mouse position on the near plane
			gluUnProject(x, y, 0.0, mm, pm, vp, &res[0],&res[1],&res[2]);
			MESH_TYPE::CoordType orig(res[0], res[1], res[2]);

			// project 2D mouse position on the far plane
			gluUnProject(x, y, 1.0, mm, pm, vp,	&res[0],&res[1],&res[2]);
			MESH_TYPE::CoordType dest(res[0], res[1], res[2]);

			FacePointer fp = NULL;
			Ray3<ScalarType> ray;		// the ray starting from the particle
			ScalarType t = 0;				// the parameter of the ray equation at which intersection occurs

			ScalarType max_dist = (dest - orig).Norm();
			CoordType dir = (dest - orig).Normalize(); 
			ray.Set(orig, dir);


			fp = vcg::tri::DoRay<MESH_TYPE, GRID>(m, g, ray, max_dist, t);// shot the ray !!

			while (fp != NULL)
			{
				face.push_back(fp);
				fp->SetD();
				fp = vcg::tri::DoRay<MESH_TYPE, GRID>(m, g, ray, max_dist, t);// shot the ray !!
			}

			for (vector<FacePointer>::iterator it = face.begin(); it != face.end(); it++)
				(*it)->ClearD();

			return face.size();
		}
		// it picks the point lying directly under the mouse cursor on the mesh surface
		// it must be called after the model-view matrix and the viewport are set
		// it returns false if no point is picked
		//
		// input:
		//		x, y: mouse 2D coordinates
		//		m: mesh from which the point is picked
		//		g: spatial grid where the mesh faces are indexed
		// output:
		//		fp: pointer to the face where the picked point lie, NULL if no point is picked
		//		q: position of the picked point
		//		bc: barycentric coordinates of the picked point relative to the face *fp
		static bool PickPoint(int x, int y, MESH_TYPE &m, GRID &g, FacePointer &fp, CoordType &q, CoordType &bc)
		{
			// draw a ray from the mouse position
			double res[3];
			GLdouble mm[16],pm[16]; GLint vp[4];
			glGetDoublev(GL_MODELVIEW_MATRIX, mm);
			glGetDoublev(GL_PROJECTION_MATRIX, pm);
			glGetIntegerv(GL_VIEWPORT, vp);

			// project 2D mouse position on the near plane
			gluUnProject(x, y, 0.0, mm, pm, vp, &res[0],&res[1],&res[2]);
			MESH_TYPE::CoordType orig(res[0], res[1], res[2]);

			// project 2D mouse position on the far plane
			gluUnProject(x, y, 1.0, mm, pm, vp,	&res[0],&res[1],&res[2]);
			MESH_TYPE::CoordType dest(res[0], res[1], res[2]);

			//		wxLogMessage(wxString::Format("ray: orig: %.2f %.2f %.2f -> dest: %.2f %.2f %.2f", 
			//			orig[0], orig[1], orig[2], dest[0], dest[1], dest[2]));

			fp = NULL;
			Ray3<ScalarType> ray;		// the ray starting from the particle
			ScalarType t = 0;				// the parameter of the ray equation at which intersection occurs

			ScalarType max_dist = (dest - orig).Norm();
			CoordType dir = (dest - orig).Normalize(); 
			ray.Set(orig, dir);

			// shot the ray !!
			fp = vcg::trimesh::DoRay<MESH_TYPE, GRID>(m, g, ray, max_dist, t);
			if (fp == NULL) return false;

			//		wxLogMessage(wxString::Format("rays: %d elems", (int)rays.size()));

			// find the intersection point
			q = ray.Origin() + dir * t;

#ifdef DEBUG_PICK
			origs.push_back(orig);
			dests.push_back(dest);
			inters.push_back(q);
#endif

			// find barycentric coordinates
			Triangle3<ScalarType> tri(fp->V(0)->P(), fp->V(1)->P(), fp->V(2)->P());
			InterpolationParameters<Triangle3<ScalarType>, ScalarType>(tri, q, bc.X(), bc.Y(), bc.Z());

			return true;
		}
	};

#ifdef DEBUG_PICK
	vector<CMesh::CoordType> GLExtendedPickTri<CMesh, CMesh::Grid>::origs;
	vector<CMesh::CoordType> GLExtendedPickTri<CMesh, CMesh::Grid>::dests;
	vector<CMesh::CoordType> GLExtendedPickTri<CMesh, CMesh::Grid>::inters;
#endif
}

#endif