#ifndef GLAREA_H
#define GLAREA_H
#pragma warning (disable: 4002)
#pragma warning (disable: 4003)
/// include headers that implement a archive in xml format
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/serialization/nvp.hpp>
/// Opengl related imports
#include <GL/glew.h>
#include <QtOpenGL/QGLWidget>
#include "mesh.h"
#include "mesh_gl.h"
#include "model.h"
#include "bvh.h"
/// wrapper imports
#include <wrap/gui/trackball.h>
#include <wrap/gl/picking.h>
#include <wrap/gl/pick.h>
#include <wrap/gl/gl_geometry.h>
#include <wrap/qt/trackball.h>
#include <fstream>
#include <QtGui>
#include <QDomDocument>
#include <Qtimer.h>
#include "mesh_importer.h"
/*! 
* \brief:
* Base class used to build the GUI and draw both BVH , mesh and Skeleton 
* update the drawing over the time 
* coded by Marco Fratarcangeli
*==========================================================================*/
class GLArea : public QGLWidget
{
	Q_OBJECT 

public:
	GLArea (QWidget * parent = 0);
	~GLArea () {for (int i = 0; i < picked.size(); i++)
		delete picked[i];
	picked.clear();
	for (int i = 0; i < selected.size(); i++)
		delete selected[i];
	selected.clear();
	}
	GLfloat shininess;
	CModel model;   // 3d model 
	BVH bvh;        // BVH 
	void addShader(GLhandleARB programObject, const GLcharARB *shaderSource, GLenum shaderType, bool linkProgram);
	void GLArea::initShader();
	void PlayAnimation(); // playing the animation
	void on_LinearBlendSkin(); // playing the animation
	bool playing; // is playing or not
	bool Snap_shot; // is playing or not
	bool	on_animation ;
	bool	on_LBS ;
	bool assign_Skel;
	bool show_Error;  // in order to show error
	bool show_Wire;  // bool veriable to show and hide the wireframe file
	bool show_bvh;  // bool veriable to show and hide the bvh file
	int frame_no;   // frame number
	int prev_frame_no;   // frame number
	QString prev_bvh;   // frame number
	QString prev_skel;   // frame number
	//bool onestepforward;   // frame number
	int stiffness_value;   // Stiffness value
	int stiffnesstr_value;   // Stiffness value
	int stiffnesstr_prevvalue;   // Stiffness value
	float stiffness_str;   // Stiffness value
	/// we choosed a subset of the avaible drawing modes (render the mesh)
	enum DrawMode{SMOOTH = 0, POINTS, WIRE, FLATWIRE, HIDDEN, FLAT, SHADER};
	public slots:
		/// widget-based user interaction slots
		void setShininess(int);
		void SetMorph(int);
		void SetFrameNo(int);
		void SetStiffness(int);
		void selectDrawMode(int mode);
		void loadModel();
		void SetStiffnessstr(int);
		void loadMesh(const QString & filePath);
		void loadSkel(const QString & filePath);
		void LoadBvh(const QString & filePath);
		//void loadTetrahedron(); // it should be able to parse Tetrahedron and load it from CGAL
		//void loadDodecahedron(); // convert back to orginal before Tetrahedron 
		void Update();
		void DrawProfilingInfo();
		void oneTimer();
		//void resetTimer();
		void Tpose();
signals:
		/// signal for setting the statusbar message
		void setStatusBar(QString message);
		void bvhLoaded();
		void meshLoaded();

protected:
	/// opengl initialization and drawing calls
	void initializeGL ();
	void resizeGL (int w, int h);
	void paintGL ();
	/// keyboard and mouse event callbacks
	void keyReleaseEvent(QKeyEvent * e);
	void keyPressEvent(QKeyEvent * e);
	void mousePressEvent(QMouseEvent*e);
	void mouseMoveEvent(QMouseEvent*e);
	void mouseDoubleClickEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent*e);
	void wheelEvent(QWheelEvent*e); 
	//void timerEvent(QTimerEvent *);
	// update camera view regard to mouse click
	void updateMouse3d();
	void rotate(const BVH::Joint* joint);
	//CMesh Skinning;// instance object of skinning class 
	void InitLight();
	void LoadVolumetricMesh(const QString & filePath);
private:
	// the active mesh instance
	//  CMeshGL	mesh_gl;
	/// the active manipulator
	QTime timer;
	vcg::Trackball track;
	/// the current drawmode
	DrawMode drawmode;
	/// mesh data structure initializer
	void initMesh(QString message);
	void ShowGlInfo();
	void DrawPlane();
	int curr_frame; // current frame
	bool show_gl_info;
	bool skel_edit_mode;	// modify the rest pose of the skeleton
	bool skel_anim_mode;	// drag the joints aroud and deform the skin
	bool onlinearblend;
	bool is_linking; ///// ???
	vector<Point3f> new_link;// new link ???
	/// is related to interface and picking ability
	Point2i mouse_pos;
	Point3f mouse_pos3;
	bool pick_on;
	bool intlocal;
	vector<Point3f*> picked;
	vector<Point3f*> selected;
	float dist;
	MeshImporter volumetricMesh;
};
#endif /*GLAREA_H_ */