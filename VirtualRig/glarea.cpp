#if defined(WIN32)
#  include <windows.h>
#  pragma warning(disable:4244)   // No warnings on precision truncation
#  pragma warning(disable:4305)   // No warnings on precision truncation
#  pragma warning(disable:4786)   // stupid symbol size limitation
#  pragma warning(disable:4273)   // No warnings
#endif
//////////////////////////////////////////////////////////////////
#include "bvh2skel.h"
#include "texture_manager.h"
#include "timer.h"
#include "extended_pick.h"
#include "particlesystem.h"
#define USE_PROFILE // for profiling
#include "profile.h"
/*************************Profiler********************************/
bool isProfilerShown =false;
bool isProfiling =false;
bool isExtendedInfo =false;
bool isSimulationFreezed = false; // if true, freese the simulation
double phys_computation_time = 0.0;
double avg_phys_computation_time = 0.0;
double tot_phys_computation_time = 0.0;
double render_computation_time = 0.0;
double avg_render_computation_time = 0.0;
double tot_render_computation_time = 0.0;
int nb_frames = 0;
int nbSimIters=1;
//bool	on_animation= false ;
/*****************************************************************/
#include "image.h"
bool isplane= true;
// create class/ struct from type plane 
struct Plane{
	// we can define plane by point and noraml 
	Vector3 _Position; // point
	Vector3 _Normal;  // normal
};
Plane ground;
//////////////////////////////////////////////
// Colors
GLfloat BBLUE[] = { 0.560784,  0.560784, 0.737255};
GLfloat WHITE[] = {0.95, 0.95, 0.95};
bool isBounBox= false;
bool isMousePress= false;
bool isthrowball= false;
// texturing 
//GLuint texture[4], dlist;                 //stores texture objects and display list
Vec3f BBoxMax;
Vec3f BBoxMin;
void LoadGLTextures(); 
GLuint texture,dlist;
//////////////////////Testuring//////////////////////////////////

#include "glarea.h"

////////////////////////// define ball ////////////////////////////
typedef struct { float X, Y,Z, Xv, Yv, Zv, radius; } ball_t; 
float bmousepostion[2]; // to translate ball
int bmousemove[2]; // to translate ball
#define MAX_VELOCITY 500 // ball velocity 
ball_t ball; 
Bvh2skel bs;
bool bvhmapped;
float animation_time = 0.0f;
double fps;
double dt;

vector<CMesh::CoordType> dpoints;
/*! 
* \brief:
* Constructor : 
* init render mode with smooth
* frame number with zero
* boolean paramter of showing the BVH to ture and load model using 
* Xml file ( xml import by boost serialization )
* naything else with false
*/
GLArea::GLArea (QWidget * parent) : QGLWidget (parent) 
{
	drawmode = SMOOTH/*FLATWIRE*/;
	frame_no = 0;
	show_bvh = true;
	//onestepforward = false;
	show_Wire = false;
	loadModel(); // load model 
	pick_on = false;
	skel_edit_mode = false;
	assign_Skel=true;
	Snap_shot= false;
	skel_anim_mode = false;
	is_linking = false;
	show_Error=false;
	on_LBS = false;
	intlocal= false;
	onlinearblend= false;
	picked.clear();
	selected.clear();
	setMouseTracking(true);
	on_animation= false ;
	playing= false ;
	stiffness_value=8;
	stiffnesstr_value=1;   // Stiffness value
	stiffnesstr_prevvalue=1;
//	stiffnestet_value=0.5;   // Stiffness value
//	stiffnesbone_value=0.5;   // Stiffness value
	stiffness_str=1.0;
	model.meshes[0]->skin.SetSNbIterations(stiffness_value);
	model.meshes[0]->skin.SetBNbIterations(stiffness_value);
	model.meshes[0]->skin.SetNbIterations(8);
	model.average_distance=0;
	//for animation
	//loadModel();
	QTimer *timer = new QTimer( this ); 
	connect( timer, SIGNAL(timeout()), this, SLOT(Update()) );
	timer->setSingleShot( false );
	timer->start(4);
	startTimer(1);
}
void GLArea::selectDrawMode(int mode)
{
	drawmode = DrawMode(mode);

	for (int i = 0; i < model.meshes.size(); i++)
		model.meshes[i]->drawmode = CMeshGL::DrawMode(mode);  // 

	updateGL();
}
/*! 
* \brief:
* load BVH : 
* which depend mainly in user click
*/
void GLArea::LoadBvh(const QString & filePath)
{
	//cout << " load bvh " ;
	bvhmapped = false;
	//cout << filePath.toStdString();
	if ( filePath == "Fat.bvh")
	{
     bvh.scalex=0.9f;
	bvh.scaley=0.6f;
	bvh.scalez=1.0f;
	}
	if ( filePath == "Slim.bvh")
	{
     bvh.scalex=0.7f;
	bvh.scaley=0.7f;
	bvh.scalez=1.0f;
	}
	if ( filePath == "Famale.bvh")
	{
     bvh.scalex=0.7f;
	bvh.scaley=0.7f;
	bvh.scalez=1.0f;
	}
	if ( filePath == "Horse.bvh")
	{
     bvh.scalex=0.2f;
	bvh.scaley=0.2f;
	bvh.scalez=0.2f;
	bvh.transx=0.0f;
	bvh.transy=0.0f;
	bvh.transz=0.0f;
	}
	if ( filePath == "lara.bvh")
	{
     bvh.scalex=0.6f;
	bvh.scaley=0.7f;
	bvh.scalez=1.0f;
	}
	if ( filePath == "woman.bvh")
	{
     bvh.scalex=0.6f;
	bvh.scaley=0.7f;
	bvh.scalez=1.0f;
	}
	if ( filePath == "Octupas.bvh")
	{
     bvh.scalex=1.3f;
	bvh.scaley=1.3f;
	bvh.scalez=1.0f;
	}
	bvh.Load(filePath.toStdString().c_str());
	model.bvhname=filePath.toStdString();
	prev_bvh=filePath;
	animation_time=0;
	emit bvhLoaded();
}
// draw plane in order to make plane collision 
void GLArea :: DrawPlane()
{ 

	glEnable(GL_DEPTH_TEST);
	/*glLightfv(GL_LIGHT0, GL_DIFFUSE, WHITE);
	glLightfv(GL_LIGHT0, GL_SPECULAR, WHITE);
	glMaterialfv(GL_FRONT, GL_SPECULAR, WHITE);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, WHITE);
	glMaterialf(GL_FRONT, GL_SHININESS, 30);*/
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	GLfloat lightPosition[] = {0, 1, 0, 1};
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosition);
	glShadeModel(GL_SMOOTH);
	glPushMatrix();
	//glPolygonMode(GL_FRONT_AND_BACK ,GL_FILL);
	glTranslatef(0.0 , BBoxMin.y(), 0.0);
	glBegin(GL_QUADS);
	glNormal3d(0, 1, 0);
	for (int x = -50; x < 50; x+=5)
	{
		for (int z = -50; z < 50; z+=5) 
		{
			glColor3f(1.0f, 1.0f,1.0f);
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE,
				(x + z) % 2 == 0 ? BBLUE : WHITE);
			glVertex3d(x, 0, z);
			glVertex3d(x+5, 0, z);
			glVertex3d(x+5, 0, z+5);
			glVertex3d(x, 0, z+5);
		}
	}
	glEnd();
	glPopMatrix();
	/// disable light
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	//glColor3f(1.0f, 1.0f,1.0f);
}
/*! 
* \brief:
* load Mesh : 
* which load volumetric mesh
*load Mesh based on the user choice */
void GLArea::loadMesh(const QString & filePath)
{
	model.meshes[0]->skin._bindparticles.clear();
	LoadVolumetricMesh(filePath);
	loadSkel(filePath);
	//bvhmapped= false;	
	playing=false ;
	on_animation=false ;
	assign_Skel=true;
	curr_frame=0;
	std::string file_bvh = filePath.toLocal8Bit().constData();
	file_bvh =file_bvh.append(".bvh");
	QString file_bvhname = file_bvh.c_str();
	LoadBvh(file_bvhname);
	prev_frame_no=frame_no;
	frame_no=0;
	model.average_distance=0;
}
void GLArea::loadSkel(const QString & filePath)
{
	model.skeleton->vert_bind.clear();
	prev_skel=filePath;
	model.skeleton->LoadNew(filePath);
}
/*! 
* \brief:
* load Mesh : 
* which load volumetric mesh
*/
void GLArea::LoadVolumetricMesh(const QString & filePath)
{
	std::string file_tet = filePath.toLocal8Bit().constData();
	file_tet =file_tet.append(".mesh");
	//cout << "\nLoadVolumetricMesh [ Particles Size] ------------> " << model.meshes[0]->skin._particles.size() << endl;
	model.meshes[0]->skin.Reset();
	volumetricMesh.Read(file_tet,model.meshes[0]->skin);
	model.meshes[0]->skin.stretch_stiffness=1.0;
	model.meshes[0]->skin.volume_stiffness=1.0;
	model.meshes[0]->skin.bone_stiffness=1.0;
	//cout << "*************************************************************";
	//cout << "\nStretch_stiffness: " <<model.meshes[0]->skin.stretch_stiffness;
	model.meshes[0]->skin.bending_stiffness=1.0;
	//cout << "\nBend_stiffness: " <<model.meshes[0]->skin.bending_stiffness;
	BBoxMin=model.meshes[0]->skin.Minboundingbox();
	BBoxMax=model.meshes[0]->skin.Maxboundingbox();
	model.meshes[0]->skin.SetGravity(Vector3(0,0,0));
	//cout << "\nAfterLoadVolumetricMesh: " << model.meshes[0]->skin._particles.size();
	//cout << "\n*************************************************************\n";
	//model.meshes[0]->skin._particles[0].isFixed=true;
	//model.meshes[0]->skin.DefineStrConstraints();
     model.meshes[0]->skin.DefineStretchConstraints();
	//model.meshes[0]->skin.DefinebendConstraints();
	model.meshes[0]->skin.DefineTetConstraints();
	/***********************************************************/
	////////////////////////// init ball ////////////////////////
	ball.X = BBoxMax.x() * bmousepostion [0] / (GLfloat)RAND_MAX; /// where the mouse now
	ball.Y = 6;
	ball.Z = 20;
	ball.radius = 1.5;
	ball.Zv = MAX_VELOCITY * (GLfloat)rand() / (GLfloat)RAND_MAX; 
	///////////////////////////////////////////////////////////////
}
/*! 
* \brief:
* load model / 3d model  : 
* using Xml file for that boost used
* load mesh and init any related materials
*/
void GLArea::loadModel()
{	
	PROFILE_SAMPLE("loadModel [include initMesh,load skeleton and load BVH]==========>>>::\n");
	//cout << " load Model  ======> ";
	//GetProfiler()->StartProfile("loadModel [include initMesh,load skeleton and load BVH]==========>>>::\n");
	//"mh_elbow.xml"
	//"mh_shoulder.xml"
	//QString fileName("data/masha.xml");
	//QString fileName("data/mh.xml");
	//QString fileName("data/mh_shoulder.xml");
	//QString fileName("data/mh_elbow.xml")
	model.average_distance=0;
	QString fileName("data/mh_male.xml");
	QFileInfo fi(fileName);
	QString dir = fi.absolutePath();
	bool res = QDir::setCurrent(dir);
	fileName = fi.fileName();
	QDomDocument doc("model_config");
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly))
		return;
	if (!doc.setContent(&file)) 
	{
		file.close();
		return;
	}
	file.close();
	//cout << "\n load Model2  ======> ";
	QDomElement docElem = doc.documentElement();
	QDomNode n = docElem.firstChild();
	while( !n.isNull() )
	{
		QDomElement e = n.toElement();
		if( !e.isNull() )
		{
			bool bOk;
			if( e.tagName() == "mesh" )
			{
				//cout << "\n load Model2  ======> ";
				CMeshGL * m = new CMeshGL();
				QString filename = e.attribute( "filename", "" );	
				//cout << "\n load Model3  ======> ";
				m->filename = filename.toStdString();
				QString vs = e.attribute( "vs", "" );				
				m->vs = vs.toStdString();
				QString fs = e.attribute( "fs", "" );				
				m->fs = fs.toStdString();
				QString color = e.attribute( "color", "" );
				if (!color.isEmpty())
				{
					QStringList slist = color.split(" ");
					assert(slist.size() == 4);
					for (int i = 0; i < slist.size(); i++)
					{
						m->color[i] = slist[i].toFloat(&bOk);
						assert(bOk);
					}
				}
				QString is_transparent = e.attribute( "is_transparent", "" );
				QString is_visible = e.attribute( "is_visible", "" );
				m->is_visible = is_visible.toInt(&bOk);

				{
					QDomNode n = e.firstChild();
					while( !n.isNull() )
					{
						QDomElement e = n.toElement();
						if( e.tagName() == "material" )
						{
							QDomNode n = e.firstChild();
							while( !n.isNull() )
							{
								QDomElement e = n.toElement();
								if( e.tagName() == "texture" )
								{
									QString filename = e.attribute( "filename", "" );
									QString type = e.attribute( "type", "" );
									//cout << filename.toStdString().c_str() << " " << type.toStdString().c_str() << endl;
									m->material.textures.insert(pair<string, string>(type.toStdString(), filename.toStdString()));
								}
								n = n.nextSibling();
							}
						}
						n = n.nextSibling();
					}

				}
				model.meshes.push_back(m);  // here load inatance of mesh
				//delete m;
			}
			if( e.tagName() == "skeleton" )
			{
				model.skeleton = new CSkeleton();
				QString filename = e.attribute( "filename", "" );	
				{
					PROFILE_SAMPLE("Load skeleton::");
					model.skeleton->filename = filename.toStdString();
				}
			}

		}

		n = n.nextSibling();
	}

	//// save model configuration
	//{
	//	std::ofstream ofs(fileName.toStdString().c_str());
	//	assert(ofs.good());
	//	boost::archive::xml_oarchive oa(ofs);
	//	oa << BOOST_SERIALIZATION_NVP(model);
	//}

	//// load model configuration
	//std::ifstream ifs(fileName.toStdString().c_str());
	//assert(ifs.good());
	//boost::archive::xml_iarchive ia(ifs);
	//ia >> BOOST_SERIALIZATION_NVP(mesh_gl);
	{
		PROFILE_SAMPLE("initMesh[ init 3D mesh using VCG lib ]::");
		initMesh("Loaded " + fileName);
		//LoadVolumetricMesh("volume"); // load volumteric model
		//LoadVolumetricMesh("MFatRes2"); // load volumteric model
		//LoadVolumetricMesh("Slim"); // load volumteric haveyman
		//LoadVolumetricMesh("Slim10516.mesh"); // load volumteric haveyman
		//LoadVolumetricMesh("Slim34049.mesh"); // load volumteric haveyman
		//LoadVolumetricMesh("Rabbit_11127.mesh"); // load volumteric haveyman
	     //LoadVolumetricMesh("lara"); // load volumteric haveyman
		//LoadVolumetricMesh("woman"); // load volumteric haveyman
		// Horse6126.mesh
		//LoadVolumetricMesh("Horse6126");
          LoadVolumetricMesh("Octupas");
		//LoadVolumetricMesh("Famale6000"); // load volumteric haveyman
		//LoadVolumetricMesh("HaveyTeamFortress2.mesh"); // load volumteric havey
		//LoadVolumetricMesh("Rabbit1.mesh"); // load volumteric havey
		//model.meshes[0]->skin.DefineStrConstraints();
		//model.meshes[0]->skin.DefinebendConstraints();
		//model.meshes[0]->skin.DefineTetConstraints(); // define the constraint
		model.skeleton->Assign(model.meshes[0]);
		//cout << "\n Number of Constraints for Bone Constraints: "
		//	<< model.meshes[0]->skin.bone_constriant << endl;
		//cout << "\n*************************************************************\n";	
	}
	{
		PROFILE_SAMPLE("loadBVH::");
		//LoadBvh("01_01.bvh");
		LoadBvh("Octupas.bvh");
	}	

	//cout << "\n Number of Over All Constraints : " << model.meshes[0]->skin._constraints.size(); 	
	//cout << "\n \n"; 
}
/*! 
* \brief:
* init mesh and related material (VCG lib)
*/
void GLArea::initMesh(QString message)
{
	string err_msg = model.Init();
	if(!err_msg.empty())
	{
		QMessageBox::warning(this,tr("Error Loading Mesh"),QString(err_msg.c_str()));
		close();
	}

	//for (int i = 0; i < model.skeleton->vert.size(); i++)
	//	dpoints.push_back(model.skeleton->vert[i]);

	show_gl_info = false;	
	emit setStatusBar(message);	
}
// QT in order to init  opengl
void GLArea::initializeGL ()
{
	PROFILE_SAMPLE("initializeGL[QtinitOpenGL]==========>>>::\n");
	glClearColor(1.0,1.0,1.0,1.0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_DEPTH_TEST);
	InitLight();
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);	// Really Nice Perspective Calculations
	glDisable(GL_BLEND);
	static bool glew_init = false;
	if (!glew_init)
	{
		GLenum res = glewInit();
		glew_init = true;
	}
	playing = false;
	curr_frame = 0;
	////////////////////plane collision ////////////////////
	// define plane 
	ground._Position=Vector3(0,BBoxMin.y()-0.08,0);
	ground._Normal=Vector3(0,1,0);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
	//cout << "\n finsih init ------------------>\n";
}
void GLArea::resizeGL (int w, int h)
{   
	glViewport (0, 0, (GLsizei) w, (GLsizei) h); 
	initializeGL();
}
void draw_picked_point(Point3f & p)
{
	glPointSize(8);
	glBegin(GL_POINTS);
	glColor4f(1, 1, 1, 1);
	glVertex(p);
	glEnd();
	//glPushMatrix();
	//glTranslate(p);
	//glutSolidSphere(2, 10, 10);
	//glPopMatrix();
}
void GLArea::rotate(const BVH::Joint* joint)
{
	int index = bs.bvh_to_skel[joint->index];

	if (index != -1)
	{
		CBone & bone = model.skeleton->bone[index];
		model.skeleton->vert[bone.child] = model.skeleton->vert[bone.node] + bone.L * bone.lchild;
	}

	for (int i = 0; i < joint->children.size(); i++)
		rotate(joint->children[i]);
}
void GLArea::Update()
{
	//GetProfiler()->StartProfile("Update[include Skin,load skeleton and load BVH]==========>>>::\n");
	//PROFILE_SAMPLE("Update[include Skin,load skeleton and load BVH]==========>>>::\n");
	{

		PROFILE_SAMPLE("FRAME::\n");
		static double past = GetRealTimeInMS();
		double now = GetRealTimeInMS();
		fps = 1000.f / (now - past);

		//onestepforward
		if ( on_animation) 
		{
			animation_time += (now - past) * 0.001f;
			frame_no = animation_time / bvh.GetInterval();
			// stop looping the bvh file
			//if ( frame_no < bvh.GetNumFrame())
			frame_no = frame_no % bvh.GetNumFrame();
			//else 
			//   isSimulationFreezed =true;
			//  cout << " here I am , this is me number of times " <<frame_no;

		}
		////////////////// ???? /////////////////////
		/*if(!isSimulationFreezed)
		{*/
		static int iters = 0;
		if (bvhmapped)
		{
			for (int i = 0; i < bs.skel_to_bvh.size(); i++)
			{
				int j = bs.skel_to_bvh[i];
				if (j == -1)
					continue;
				model.skeleton->bone[i].L = bvh.GetJoint(j)->L; // local corrdinate
				//if (iters == 0)
				//{
				//	model.skeleton->InitLocalChilds();
				//	iters++;
				//}
			}

			//		rotate(bvh.GetJoint(0));

			for (int i = 0; i < model.skeleton->bone.size(); i++)
			{
				//Matrix33f m;
				//m.SetIdentity();
				int j = bs.skel_to_bvh[i];
				assert(j != -1);

				//			model.skeleton->RotateBone(bvh.GetJoint(j)->mrot, &(model.skeleton->bone[i]), &(model.skeleton->bone[i]));

				//int node_idx = model.skeleton->bone[i].node;
				//int child_idx = model.skeleton->bone[i].child;
				//Point3f & node = model.skeleton->vert[node_idx];
				//Point3f & child = model.skeleton->vert[child_idx];

				//child -= node;
				//child = bvh.GetJoint(j)->mrot * child;
				//child += node;
			}
		}
		past = now;
		////////////////////// Profiling ////////////////////////
		if (nb_frames == 0)
			tot_phys_computation_time = 0.0;	
		nb_frames++;
		//cout << " \n \n hereeeeeeee " <<  model.count;
		if (!isSimulationFreezed)
		{

			{
				//PROFILE_SAMPLE("TimeStep::Physics Simulation==========>>>::\n");
				PROFILE_SAMPLE("PHYSICS SIMULATION::");
				double start = GetRealTimeInMS(); // for update phys_simulation
				if (on_animation)
				{//model.meshes[0]->skin.TimeStep();
					//current_frames ++;
					//model.meshes[0]->skin.SetGravity(Vector3(0,-100,0));
					if(onlinearblend)
					{
						model.updateLinearSkin(1,1,1);	
					     model.meshes[0]->skin.ComputeNormals();
					}
					else
					{
					     model.updateLinearSkin(stiffness_value,stiffnesstr_value,stiffnesstr_prevvalue);
						model.Update(on_animation,stiffness_value);
						//++model.LinearSmoothSkin(model.meshes[0]);
						model.meshes[0]->skin.ComputeNormals();
						stiffnesstr_prevvalue=stiffnesstr_value;
					}
					//model.Update();
				}
				//	else
				//	for (int i = 0; i < nbSimIters; i++)    ////// this mean 
				//	model.Update(on_animation);
				//model.Update();

				double end = GetRealTimeInMS();
				phys_computation_time = end - start;
				tot_phys_computation_time += phys_computation_time;
				assert(nb_frames != 0);
				avg_phys_computation_time = tot_phys_computation_time / (double) nb_frames;
			}
		}	
		//model.meshes[0]->skin.CollisionPlane(ground._Position ,ground._Normal);

	}

	// update the stiffness of the mesh and number of iteration
	/*float temp_stiffness;
	model.meshes[0]->skin.stretch_stiffness=stiffness_str;
	temp_stiffness= (stiffness_value * 1.0)/64;
	model.meshes[0]->skin.stretch_stiffness=temp_stiffness;
	model.meshes[0]->skin.SetSNbIterations(stiffness_value);
	model.meshes[0]->skin.SetBNbIterations(stiffness_value);
	model.meshes[0]->skin.SetNbIterations(8); */
	updateGL();

}
void GLArea::paintGL ()
{
	//GetProfiler()->StartProfile("Render[include Skin,load skeleton and load BVH]==========>>>::\n");
	//PROFILE_SAMPLE("Render[include Skin,load skeleton and load BVH]==========>>>::\n");
	{
		PROFILE_SAMPLE("RENDER::");
		double start = GetRealTimeInMS(); // for rendering 
		double t0 = GetRealTimeInMS();

		if (!show_gl_info)
			ShowGlInfo();
		//glColor3f(0, 0, 1);
		glClearStencil(0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45, GLArea::width()/(float)GLArea::height(), 0.01, 85);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		gluLookAt(0,0,35,   0,0,0,   0,1,0);
		track.center=vcg::Point3f(0, 0, 0);
		track.radius= 8;
		track.GetView();
		track.Apply();
		timer.restart(); 
		glPushMatrix();
		float d ;
		d = 4.5f / model.meshes[0]->bbox.Diag();
		////	float d = 2.5f / model.skeleton->bbox.Diag();
		if (!model.meshes[0]->morphs.empty())
			d = 1.0f / model.meshes[0]->morphs[0]->bbox.Diag();
		vcg::glScale(d);
		if (model.meshes[0]->morphs.empty())
			glTranslate(-model.meshes[0]->bbox.Center());	
		else
		{
			glTranslate(-model.meshes[0]->morphs[0]->bbox.Center());
			glTranslatef( 0, model.meshes[0]->morphs[0]->bbox.Diag() / 20, 0);
		}
		////	glTranslate(-model.skeleton->bbox.Center());	
		//	if (!isSimulationFreezed)
		bvh.RenderFigure( frame_no);
		//else
		//	{
		//	bvh.RenderFigure(0);
		//model.Update(1);
		//	}
		/* the plane inside with exactly will be under the foot*/
		if (show_Error)
		{    
			show_bvh=!show_Error;
			isplane=!show_Error;
			//isplane=!isplane;
			glClearColor(1, 1, 1, 1);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glMatrixMode(GL_PROJECTION);
			glLoadIdentity();
			gluPerspective(35, GLArea::width()/(float)GLArea::height(), 0.01, 85);
			glMatrixMode(GL_MODELVIEW);
			glLoadIdentity();
			gluLookAt(0,0,32,   0,0,0,   0,1,0);
			track.center=vcg::Point3f(0, 0, 0);
			track.radius= 8;
			track.GetView();
			track.Apply();
			timer.restart(); 
		}
		else
		{
			isplane=!show_Error;
			//show_bvh=!show_Error;
			glClearColor(1.0, 1.0, 1.0,1.0); }
		{
			if (isplane)
			{
				//DrawPlane();
				//DrawWall();
				//DrawWall1();
				//DrawWall2();
				//glColor3f(1.0f, 1.0f,1.0f);
			}
			if (show_bvh)
			{
				glPointSize(10.f);
				glBegin(GL_POINTS);
				for (int i = 0; i < bvh.GetNumJoint(); i++)
				{
					const BVH::Joint * j = bvh.GetJoint(i);
					glVertex3f(j->wc[0], j->wc[1], j->wc[2]) ;
				}
				glEnd();

				for (int i = 0; i < bvh.GetNumJoint(); i++)
				{
					const BVH::Joint * j = bvh.GetJoint(i);

					glPushMatrix();
					glTranslatef(j->wc[0], j->wc[1], j->wc[2]);

					glBegin(GL_LINES);
					glColor3f(1, 0, 0);
					glVertex3f(0, 0, 0);
					glVertex(j->L.GetColumn(0));
					glColor3f(0, 1, 0);
					glVertex3f(0, 0, 0);
					glVertex(j->L.GetColumn(1));
					glColor3f(0, 0, 1);
					glVertex3f(0, 0, 0);
					glVertex(j->L.GetColumn(2));
					glEnd();
					glPopMatrix();
				}
			}

			// draw the mapping between bvh and skeleton
			{
				if (!bvhmapped)
				{
					BVH::transx = 0;
					BVH::transy = 0;
					BVH::transz = 0;

					bvh.RenderFigure(0.0);
					Box3f bbox;

					for (int i = 0; i < bvh.GetNumJoint(); i++)
					{
						const BVH::Joint * j = bvh.GetJoint(i);
						bbox.Add(Point3f(j->wc[0], j->wc[1], j->wc[2]));
					}

					Point3f v = model.meshes[0]->bbox.Center() - bbox.Center();

					//cout << "mesh " <<  model.meshes[0]->bbox.Center()[0] << " " << model.meshes[0]->bbox.Center()[1] << " " << model.meshes[0]->bbox.Center()[2] << endl;
					//cout << "bbox " <<  bbox.Center()[0] << " " << bbox.Center()[1] << " " << bbox.Center()[2] << endl;
					//cout << "v " <<  v[0] << " " << v[1] << " " << v[2] << endl;

					BVH::transx = v[0];
					BVH::transy = v[1];
					BVH::transz = v[2];
					//		{
					//			PROFILE_SAMPLE("RenderBVH::");
					bvh.RenderFigure(0.0);
					//		}
					//		{
					//			PROFILE_SAMPLE("MapBVHToSkel::");
					bs.MapBvhToSkel(&bvh, model.skeleton);
					//		}
					bvhmapped = true;
				}

				if (show_bvh)
				{
					glBegin(GL_LINES);
					for (int i = 0; i < bs.skel_to_bvh.size(); i++)
					{
						int j = bs.skel_to_bvh[i];
						if (j == -1)
							continue;
						glVertex(model.skeleton->vert[model.skeleton->bone[i].node]);
						glVertex(bvh.GetJoint(j)->wc);
					}
					glEnd();
				}
			}

			//GLfloat lpos[4] = {50, 200, 150, 1};
			//glLightfv(GL_LIGHT0, GL_POSITION, lpos);

			// DISEGNA SCHELETTRO QUI
			// draw the skeleton 

			if ((pick_on) && skel_anim_mode)
			{

				glDisable(GL_BLEND);
				glEnable(GL_DEPTH_TEST);
				glDepthMask(1);
				glDepthFunc( GL_LEQUAL ); 
				//	{
				//		PROFILE_SAMPLE("Draw Skeleton ::")
				model.skeleton->Draw(true);	// write the skeleton in the depth buffer
				//	}
				int res = Pick(	mouse_pos[0], height() - mouse_pos[1], model.skeleton->vert, picked, draw_picked_point, 4, 4);
				if (res > 0)
				{
					glReadBuffer(GL_FRONT);
					glReadPixels(mouse_pos[0], height() - mouse_pos[1], 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &dist);

					//for (int x = 0; x < width(); x++)
					//{
					//	for (int y = 0; y < height(); y++)
					//	{
					//		glReadPixels(x, height() - y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &dist);
					//		printf("%d %d :%.4f\n", x, y, dist); 
					//		if (dist != 1.f)
					//			break;
					//	}
					//	printf("\n");
					//}
				}

				//	printf("x: %d\ty: %d\tres: %d\tdist: %.3f\n", mouse_pos[0], height() - mouse_pos[1], res, dist);
				pick_on = false;	

			}

			if (pick_on && skel_edit_mode)
			{
				/*	vector<CMesh::FacePointer> picked_faces;
				int nres = GLExtendedPickTri<CMesh, CMesh::SpatialGridType>::PickFaces(mouse_pos[0], height() - mouse_pos[1], *model.meshes[0], model.meshes[0]->spatial_grid, picked_faces);
				if (nres > 0)
				{
				CMesh::CoordType p(0, 0, 0);
				for (int i = 0; i < nres; i++)
				{
				CFace & f = *picked_faces[i];
				for (int j = 0; j < 3; j++)
				p += f.P(j);
				}
				p /= nres * 3;
				dpoints.push_back(p);
				if (!model.skeleton)
				{
				model.skeleton = new CSkeleton();

				//			{
				//				PROFILE_SAMPLE("Draw Link::");
				model.skeleton->AddLink(p);
				//			}
				}
				}

				cout << "picked points: " << dpoints.size() << endl;
				pick_on = false;	
				*/}

			if (!dpoints.empty())
			{
				glEnable(GL_BLEND);
				glDisable(GL_DEPTH_TEST);
				glDisable(GL_LIGHTING);

				glPointSize(6);
				if (skel_edit_mode)
				{
					if (/*!selected.empty() && */is_linking)
					{
						int res = Pick(	mouse_pos[0], height() - mouse_pos[1], dpoints, selected, draw_picked_point);
						//			cout << "selected points: " << selected.size() << endl;
						if (!selected.empty())
						{
							if (new_link.empty())
								new_link.push_back(*selected.back());
							else if (new_link.size() == 1)
							{
								if (*selected.back() != new_link[0])
									new_link.push_back(*selected.back());
							}
							else if (new_link.size() == 2)
							{
								model.skeleton->AddLink(new_link[0], new_link[1]);
								new_link.clear();
								is_linking = false;
							}
						}
					}
				}
				glBegin(GL_POINTS);
				glColor3f(1, 0, 0);		
				for (int i = 0; i < dpoints.size(); i++)			
					glVertex(dpoints[i]);
				glColor3f(0, 1, 0);		
				for (int i = 0; i < selected.size(); i++)			
					glVertex(*selected[i]);
				glEnd();

				//if (!new_link.empty())
				//{
				//	glBegin(GL_LINES);
				//	glVertex(new_link[0]);
				//	if (new_link.size() == 1)
				//		glVertex(mouse_pos3);
				//	//else
				//	//	glVertex(new_link[1]);
				//	glEnd();
				//}
				glDisable(GL_BLEND);
				glEnable(GL_DEPTH_TEST);
			}

			if (!picked.empty())
			{
				glPushAttrib(GL_ENABLE_BIT | GL_CURRENT_BIT);

				glColor4f(0, 1, 0, 1);
				glPointSize(8.f);
				glBegin(GL_POINTS);
				glVertex(*picked[0]);
				glEnd();

				glColor4f(0, 1, 0, 1);
				glPointSize(8.f);
				glBegin(GL_LINES);
				glVertex(*picked[0]);
				glVertex(mouse_pos3);
				glEnd();

				glPopAttrib();

				picked[0]->X() = mouse_pos3[0];
				picked[0]->Y() = mouse_pos3[1];
				picked[0]->Z() = mouse_pos3[2];
			}
			updateMouse3d();
			/*{
			PROFILE_SAMPLE("TimeStep::Physics Simulation==========>>>::\n");
			model.Update();
			}*/
			///////////////////// Draw model ////////////////////
			// cout << " here --------------> " <<show_Wire;
			model.Draw(show_Error, show_Wire);
			//	shp->Bind();
			/*
			// the trimesh drawing calls
			switch(drawmode)
			{
			case SMOOTH: 
			//		glw.Draw<vcg::GLW::DMSmooth,   vcg::GLW::CMNone, GLW::TMPerWedgeMulti>();
			glw.Draw<vcg::GLW::DMSmooth,   vcg::GLW::CMPerVert, vcg::GLW::TMNone> ();
			break;
			case POINTS: 
			glw.Draw<vcg::GLW::DMPoints,   vcg::GLW::CMNone,vcg::GLW::TMNone> ();
			break;
			case WIRE: 
			glw.Draw<vcg::GLW::DMWire,     vcg::GLW::CMNone,vcg::GLW::TMNone> ();
			break;
			case FLATWIRE: 
			glw.Draw<vcg::GLW::DMFlatWire, vcg::GLW::CMNone, GLW::TMPerWedgeMulti> ();
			break;
			case HIDDEN: 
			glw.Draw<vcg::GLW::DMHidden,   vcg::GLW::CMNone,vcg::GLW::TMNone> ();
			break;
			case FLAT: 
			glw.Draw<vcg::GLW::DMFlat,     vcg::GLW::CMNone,vcg::GLW::TMNone> ();
			break;
			default: 
			break;
			}
			*/
			//	shp->Unbind();

			glPopMatrix();

			//	track.DrawPostApply();
			if (playing)
			{
				model.ApplyMorph(curr_frame);


				QString filename;
				filename = QString(model.anim_dir.c_str()) + QString("_seq_") + QString::number(curr_frame) + QString(".png");
				grabFrameBuffer().save(filename);
				curr_frame++;
				if (curr_frame >= model.GetNbMorphs())
					playing = false;
			}
			else if (show_Error)
			{ 
				glColor4f(1.0, 0.0, 0.0,1);
				renderText ( 10, 30, QString("Estimated Error from Red to Blue : where red is high value"));	
				// renderText(10, 30, QString("[ N ] show / hide profiler:\n"));	
			}	
			else if (0)
			{
				double t1 = GetRealTimeInMS();
				double end = GetRealTimeInMS();
				render_computation_time = end - start;
				tot_render_computation_time += phys_computation_time;
				//assert(nb_frames != 0);
				avg_render_computation_time = tot_render_computation_time / (double) nb_frames;
				dt = t1 - t0;
				glColor4f(0.0, 0.0, 0.0, 1);
				renderText ( 10, 10, QString("fps: %1\t\tdt: %2").arg(fps).arg(dt));
				renderText(10, 30, QString("[ N ] show / hide profiler:\n"));	
				renderText(10, 70, QString("[Physics Simulation]\n"));	
				renderText(10, 90,QString("phys_t: %1 ms").arg
					(phys_computation_time));
				renderText(10, 110,QString("avg: %1 ms").arg
					(avg_phys_computation_time));
				renderText(10, 130,QString("frame: %1").arg
					(nb_frames));
				renderText(10, 150, QString("[Rendering]\n"));	
				renderText(10, 170,QString("render_t: %1 ms").arg
					(render_computation_time));
				renderText(10, 190,QString("avg: %1 ms").arg
					(avg_render_computation_time));
				renderText(10, 210,QString("frame: %1").arg
					(nb_frames));
			}
			if (isProfilerShown)
			{
				renderText(700, 30, QString("[ N ] Profiler:\n"));	
				DrawProfilingInfo();
			}


		}
	}
	//glutSwapBuffer();
} 
/*
glPrint(c, 10, y, 0, "phys_t: %.3f ms   avg: %.3f ms  frame: %d   comp: %s", 
phys_computation_time, avg_phys_computation_time, nb_frames, comp.c_str());  y += 20;
*/
void GLArea::PlayAnimation()
{
	{
		PROFILE_SAMPLE("PlayAnimation::\n");
		playing = true;
		on_animation= true;
		Snap_shot= false;
		intlocal=true;
		//		onestepforward=!onestepforward;
		curr_frame = 0; // start from frame one
		//model.lines_visible =true;
		//model.skeleton->InitLocalChilds();
		 model.average_distance=0;
		 model.LinearSkin(model.meshes[0]);
		 model.skeleton->skel_average_distance=model.average_distance;
		if (on_LBS)
		{
			on_LinearBlendSkin();
			assign_Skel=false;

		}
		else if (assign_Skel)
		{
			assign_Skel=false;
			// assgin is done
		     model.skeleton->skel_average_distance=model.average_distance;
			model.skeleton->Assign(model.meshes[0]);
			//model.Saveobj();
		}

	}
}
void GLArea::oneTimer()
{   

	playing = false;
	Snap_shot= true;
	on_animation= false;
	PROFILE_SAMPLE("FRAME::\n");
	if (assign_Skel)
	{
		assign_Skel=false;
		model.skeleton->Assign(model.meshes[0]);
	}
	static double past = GetRealTimeInMS();
	double now = GetRealTimeInMS();
	fps = 1000.f / (now - past);
	animation_time += (now - past) * 0.000001f;
	frame_no = animation_time / bvh.GetInterval();
	// stop looping the bvh file
	//if ( frame_no < bvh.GetNumFrame())
	frame_no = frame_no % bvh.GetNumFrame();
	static int iters = 0;
	if (bvhmapped)
	{
		for (int i = 0; i < bs.skel_to_bvh.size(); i++)
		{
			int j = bs.skel_to_bvh[i];
			if (j == -1)
				continue;
			model.skeleton->bone[i].L = bvh.GetJoint(j)->L; // local corrdinate
		}

		//		rotate(bvh.GetJoint(0));

		for (int i = 0; i < model.skeleton->bone.size(); i++)
		{
			//Matrix33f m;
			//m.SetIdentity();
			int j = bs.skel_to_bvh[i];
			assert(j != -1);

			//			model.skeleton->RotateBone(bvh.GetJoint(j)->mrot, &(model.skeleton->bone[i]), &(model.skeleton->bone[i]));

			//int node_idx = model.skeleton->bone[i].node;
			//int child_idx = model.skeleton->bone[i].child;
			//Point3f & node = model.skeleton->vert[node_idx];
			//Point3f & child = model.skeleton->vert[child_idx];

			//child -= node;
			//child = bvh.GetJoint(j)->mrot * child;
			//child += node;
		}
	}
	past = now;
	////////////////////// Profiling ////////////////////////
	if (nb_frames == 0)
		tot_phys_computation_time = 0.0;	
	nb_frames++;
	//cout << " \n \n hereeeeeeee " <<  model.count;
	if (!isSimulationFreezed)
	{

		{
			//PROFILE_SAMPLE("TimeStep::Physics Simulation==========>>>::\n");
			PROFILE_SAMPLE("PHYSICS SIMULATION::");
			double start = GetRealTimeInMS(); // for update phys_simulation
			if (1)
			{//model.meshes[0]->skin.TimeStep();
				//current_frames ++;
				//model.meshes[0]->skin.SetGravity(Vector3(0,-100,0));
				model.Update(1,stiffness_value);

				//model.Update();
			}
			//	else
			//	for (int i = 0; i < nbSimIters; i++)    ////// this mean 
			//	model.Update(on_animation);
			//model.Update();

			double end = GetRealTimeInMS();
			phys_computation_time = end - start;
			tot_phys_computation_time += phys_computation_time;
			assert(nb_frames != 0);
			avg_phys_computation_time = tot_phys_computation_time / (double) nb_frames;
		}
	}	
	model.meshes[0]->skin.CollisionPlane(ground._Position ,ground._Normal);
	// take snap shot 
	if (Snap_shot)
	{
		model.ApplyMorph(curr_frame);


		QString filename;
		filename = QString(model.anim_dir.c_str()) + QString("_seq_") + QString::number(curr_frame) + QString(".png");
		grabFrameBuffer().save(filename);
		curr_frame++;
		if (curr_frame >= model.GetNbMorphs())
			playing = false;
	}
	// updateGl
	//updateGL();

}
void GLArea ::on_LinearBlendSkin()
{  


	if(!onlinearblend)
	{
	     model.LinearSkin(model.meshes[0]);
		//model.LinearSmoothSkin(model.meshes[0]);
		model.skeleton->InitLocalChilds();
		onlinearblend= true ;
	}
	else
		model.updateLinearSkin(1,1,1);
	//model.skeleton->Assign(model.meshes[0]);
	//model.skeleton->InitLocalChilds();


}
void GLArea::Tpose()
{	
	playing=false ;
	on_animation=false ;
	frame_no=0;
	curr_frame=0;
	prev_frame_no=0;
	model.skeleton->bindpose();
	model.meshes[0]->skin.setBindPose();
	//  model.meshes[0]->skin.ComputeNormals();
	assign_Skel=true;
	bvh.Clear();
	LoadBvh(prev_bvh);
	//loadSkel(prev_skel);
	//model.skeleton->InitLocalChilds();
	updateGL();
}
void GLArea::keyReleaseEvent (QKeyEvent * e)
{
	e->ignore ();

	//////// Display Profile ///////
	if (e->key () == Qt::Key_N)
	{
		cout << "Profile::OFF " << endl;
		isProfilerShown = !isProfilerShown;
		isProfiling = isProfilerShown;
		isExtendedInfo = isProfilerShown;
	}
	if (isProfiling) 
		GetProfiler()->Reset();

	////////////////////////////////////////////
	// number of frames equal zero
	nb_frames = 0; 
	updateGL ();
}
void GLArea::keyPressEvent (QKeyEvent * e)
{
	e->ignore ();
	if (e->key () == Qt::Key_Control)
		track.ButtonDown (QT2VCG (Qt::NoButton, Qt::ControlModifier));
	if (e->key () == Qt::Key_Shift)
		track.ButtonDown (QT2VCG (Qt::NoButton, Qt::ShiftModifier));
	if (e->key () == Qt::Key_Alt)
		track.ButtonDown (QT2VCG (Qt::NoButton, Qt::AltModifier));

	if ((e->key () == Qt::Key_Shift) && skel_edit_mode)
	{
		cout << "linking activated" << endl;
		is_linking = true;
	}

	updateGL ();
}
void GLArea::mousePressEvent (QMouseEvent * e)
{

	e->accept ();
	setFocus ();
	if ((e->button() == Qt::RightButton) && (skel_edit_mode || skel_anim_mode))
		pick_on = true;

	track.MouseDown (e->x (), height () - e->y (), QT2VCG (e->button (), e->modifiers ()));
	mouse_pos[0] = e->x();
	mouse_pos[1] = e->y();

	//	cout << "mousePressEvent " << mouse_pos[0] << " " << mouse_pos[1] << endl;
	updateGL ();
}
void GLArea ::mouseDoubleClickEvent(QMouseEvent *e)
{
	if (e-> type() == QEvent::MouseButtonDblClick)      
	{ 

		QMouseEvent * mouseEvent = static_cast <QMouseEvent *> (e);
		if (mouseEvent -> button() == Qt::LeftButton)
		{
			isMousePress=true;
			float pos_x = (float)mouseEvent->posF().x();
			float pos_y =(float) mouseEvent->posF().y();
			this->updateGL();
			bmousepostion[0]=pos_x;
			bmousepostion[1]=pos_y;
			cout << bmousepostion [0] << "  " << bmousepostion[1];
			updateGL ();
		}
	}

}
void GLArea::mouseMoveEvent (QMouseEvent * e)
{
	if (e->buttons ()) {
		track.MouseMove (e->x (), height () - e->y ());
	}
	//		cout << "mouseMoveEvent " << mouse_pos[0] << " " << mouse_pos[1] << endl;
	updateGL ();
}
void GLArea::mouseReleaseEvent (QMouseEvent * e)
{
	track.MouseUp (e->x (), height () - e->y (), QT2VCG (e->button (), e->modifiers ()));

	mouse_pos[0] = e->x();
	mouse_pos[1] = e->y();

	if (e->button() == Qt::RightButton)
	{
		picked.clear();
	}	

	//	cout << "mouseReleaseEvent " << mouse_pos[0] << " " << mouse_pos[1] << endl;

	updateGL ();
}
void GLArea::wheelEvent (QWheelEvent * e)
{
	const int WHEEL_STEP = 120;
	track.MouseWheel (e->delta () / float (WHEEL_STEP), QTWheel2VCG (e->modifiers ()));
	updateGL ();
}
void GLArea::ShowGlInfo()
{
#if 0

	LOAD TEXTURES WITH QT
		if (true)//(hasToUpdateTexture)
		{
			//      foreach (MeshModel *mp,this->md()->meshList)
			mesh_gl.glw.TMId.clear();

			// qDebug("Beware: deleting the textures could lead to problems for shared textures.");
			//		hasToUpdateTexture = false;
		}

		//    foreach (MeshModel *mp, this->md()->meshList)
		{
			if(! mesh_gl.textures.empty() && mesh_gl.glw.TMId.empty())
			{
				GLint MaxTextureSize;
				glGetIntegerv(GL_MAX_TEXTURE_SIZE,&MaxTextureSize);
				printf("maximal allowed texture size is : %d\n", MaxTextureSize);

				for(unsigned int i = 0; i < mesh_gl.textures.size(); ++i)
				{
					GLuint id = -1;
					bool res = TextureManager::Inst()->Load(mesh_gl.textures[i].c_str(), id);
					if (!res) 
						printf("Failure of loading texture %s\n", mesh_gl.textures[i].c_str());
					mesh_gl.glw.TMId.push_back(id);

					//glEnable(GL_TEXTURE_2D);
					//QImage img, imgScaled, imgGL;

					//bool res = img.load(mesh_gl.textures[i].c_str());

					//if(!res)
					//{
					//	// Note that sometimes (in collada) the texture names could have been encoded with a url-like style (e.g. replacing spaces with '%20') so making some other attempt could be harmless
					//	QString ConvertedName = QString(mesh_gl.textures[i].c_str()).replace(QString("%20"), QString(" "));
					//	res = img.load(ConvertedName);
					//	if(!res) qDebug("Failure of loading texture %s", mesh_gl.textures[i].c_str());
					//	else qDebug("Warning, texture loading was successful only after replacing %%20 with spaces;\n Loaded texture %s instead of %s",qPrintable(ConvertedName),mesh_gl.textures[i].c_str());
					//}
					//if(!res && QString(mesh_gl.textures[i].c_str()).endsWith("dds",Qt::CaseInsensitive))
					//{
					//	qDebug("DDS binding!");
					//	int newTexId = bindTexture(QString(mesh_gl.textures[i].c_str()));
					//	mesh_gl.glw.TMId.push_back(newTexId);
					//}
					//else
					//{
					//	// image has to be scaled to a 2^n size. We choose the first 2^N <= picture size.
					//	int bestW=pow(2.0,floor(::log(double(img.width() ))/::log(2.0)));
					//	int bestH=pow(2.0,floor(::log(double(img.height()))/::log(2.0)));
					//	while(bestW>MaxTextureSize) bestW /=2;
					//	while(bestH>MaxTextureSize) bestH /=2;

					//	printf("Loading textures\n");
					//	printf("Texture[ %3i ] =  '%s' ( %6i x %6i ) -> ( %6i x %6i )\n",	i, mesh_gl.textures[i].c_str(), img.width(), img.height(), bestW, bestH);
					//	//log->Log(GLLogStream::SYSTEM,"Loading textures");
					//	//log->Logf(GLLogStream::SYSTEM,"	Texture[ %3i ] =  '%s' ( %6i x %6i ) -> ( %6i x %6i )",	i,mp->cm.textures[i].c_str(), img.width(), img.height(),bestW,bestH);
					//	imgScaled = img.scaled(bestW, bestH, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
					//	imgGL = convertToGLFormat(imgScaled);
					//	mesh_gl.glw.TMId.push_back(0);

					//	glGenTextures( 1, (GLuint*)&(mesh_gl.glw.TMId.back()) );
					//	glBindTexture( GL_TEXTURE_2D, mesh_gl.glw.TMId.back() );
					//	glTexImage2D( GL_TEXTURE_2D, 0, 3, imgGL.width(), imgGL.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, imgGL.bits() );
					//	gluBuild2DMipmaps(GL_TEXTURE_2D, 3, imgGL.width(), imgGL.height(), GL_RGBA, GL_UNSIGNED_BYTE, imgGL.bits() );
					//	qDebug("      	will be loaded as GL texture id %i  ( %i x %i )", mesh_gl.glw.TMId.back(), imgGL.width(), imgGL.height());
					//}

					////if (glas.textureMagFilter == 0 ) 	
					////	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
					////else	
					//	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

					////if (glas.textureMinFilter == 0 ) 	
					////	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
					////else	
					//	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );

					//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
				}
			}
			//		glDisable(GL_TEXTURE_2D);
		}

#endif

		GLint MaxTextureSize;
		glGetIntegerv(GL_MAX_TEXTURE_SIZE,&MaxTextureSize);
		printf("maximal allowed texture size is : %d\n", MaxTextureSize);
		int n;
		glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &n);
		printf ("GL_MAX_VERTEX_ATTRIBS: %d\n", n);

		show_gl_info = true;
		pick_on = false;
		picked.clear();
}
void GLArea::SetMorph(int index)
{
	model.ApplyMorph(index);
}
void GLArea::SetFrameNo(int v)
{
	frame_no = v;
}
void GLArea::SetStiffness(int v)
{
	stiffness_value = v/10;
	updateGL();
	//cout << "\n the Stiffness value" <<stiffness_value ;
}

void GLArea::SetStiffnessstr(int v)
{
	stiffnesstr_prevvalue=stiffnesstr_value;
	stiffnesstr_value = v;
	updateGL();
	//cout << "\n the Stiffness value" <<stiffness_value ;
}
void GLArea::setShininess(int val)
{
	//shininess = val / 10.f;
	//shp->prog.Bind();
	//shp->prog.Uniform("shininess", shininess);
	//shp->prog.Unbind();

	//printf("%.4f\n", shininess);

	updateGL();
}
void GLArea::updateMouse3d()
{
	GLdouble p[3];
	vcg::Matrix44d m, prm;
	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX, &m[0][0]);
	glGetDoublev(GL_PROJECTION_MATRIX, &prm[0][0]);

	gluUnProject(mouse_pos[0],  height() - mouse_pos[1], dist, &m[0][0], &prm[0][0], viewport, &p[0], &p[1], &p[2]);

	mouse_pos3 = Point3f(p[0], p[1], p[2]);
	//printf("%d\t%d\n",				mouse_pos[0],  mouse_pos[1]);
	//printf("%.3f\t%.3f\t%.3f\n",	mouse_pos3[0], mouse_pos3[1], mouse_pos3[2]);
}
void GLArea::DrawProfilingInfo()
{


	float x =600;
	float y =50;
	if (isProfiling)
	{
		if (isProfilerShown)
		{

			std::stringstream op;
			GetProfiler()->GetRootNode()->DisplayFlatStats(op);
			char msg[256] = " ";
			while (strlen(msg) > 0)
			{
				op.getline(msg, 256);
				//cout << msg ;
				glColor4f(1, 1, 1, 1);
				renderText(x, y, QString(msg));
				//qglColor(QColor(255,255,255));
				// print using qt command
				//glPrint(c, x, y, 0, "%s", msg); y += 20;
				y+=20;
			}
		}
	}

}
void GLArea::InitLight() 
{
	// Set the last component of the position to 0 to indicate
	// a directional light source

	GLfloat position[4] = { 30,30,100, 1};
	GLfloat diffuse[4] = { 0.75,0.75,0.75,1};
	GLfloat specular[4] = { 0,0,0,1};
	GLfloat ambient[4] = { 0.2, 0.2, 0.2, 1.0 };

	GLfloat zero[4] = {0,0,0,0};
	glLightfv(GL_LIGHT1, GL_POSITION, position);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, specular);
	glLightfv(GL_LIGHT1, GL_AMBIENT, zero);
	glEnable(GL_LIGHT1);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glEnable(GL_COLOR_MATERIAL);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

	GLfloat spec_mat[4] = {1,1,1,1};
	float glexponent = 30;
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, &glexponent);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spec_mat);

	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	float back_color[] = { 0.0,0.0,1.0,1};
	glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, back_color);
	glEnable(GL_LIGHT1);
}
void LoadGLTextures() {	

	/*	Image *image3;
	image3 = (Image *) malloc(sizeof(Image));
	if (image3 == NULL) {
	printf("Error allocating space for image");
	exit(0);
	}
	/* Load Texture*/
	/*if (!ImageLoad("sss.bmp", image3)) { 
	exit(1);
	} 
	/* Create Texture	*****************************************/
	/*glTexImage2D(GL_TEXTURE_2D, 0, 3, image3->sizeX, image3->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, image3->data);
	glBindTexture(GL_TEXTURE_2D, texture);   /* 2d texture (x and y size)*/

	/*glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR); /* scale linearly when image bigger than texture*/
	/*glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR); /* scale linearly when image smalled than texture*/
	/*glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_REPEAT);
	//cout << "\n \n till here working\n\n\n ";


	free(image3->data);
	free(image3);*/


}
void GLArea::initShader()
{
	/*
	if(!glh_init_extensions("GL_ARB_shader_objects GL_ARB_vertex_shader GL_ARB_fragment_shader"))
	{
	cout << "Necessary extensions unsupported: " << glh_get_unsupported_extensions() << endl;
	quitapp(-1);
	}

	// define search path for simple_lighting.glsl file
	data_path media;
	media.path.push_back(".");
	media.path.push_back("../../../MEDIA");
	media.path.push_back("../../../../MEDIA");
	media.path.push_back("../../../../../../../MEDIA");

	string filename = media.get_file("programs/glsl_simple_lighting/vertex_lighting.glsl");
	if (filename == "")
	{
	printf("Unable to locate vertex_lighting.glsl, exiting...\n");

	}

	// load and create vertex lighting program
	vertexLighting = glCreateProgramObjectARB();

	GLcharARB *shaderData = read_text_file(filename.c_str());
	addShader(vertexLighting, shaderData, GL_VERTEX_SHADER_ARB, true);

	filename = media.get_file("programs/glsl_simple_lighting/passthrough_vp.glsl");
	if (filename == "")
	{
	printf("Unable to locate passthrough_vp.glsl, exiting...\n");

	}

	// load and create fragment lighting program
	fragmentLighting = glCreateProgramObjectARB();

	shaderData = read_text_file(filename.c_str());
	addShader(fragmentLighting, shaderData, GL_VERTEX_SHADER_ARB, false);

	filename = media.get_file("programs/glsl_simple_lighting/fragment_lighting.glsl");
	if (filename == "")
	{
	printf("Unable to locate fragment_lighting.glsl, exiting...\n");

	}

	shaderData = read_text_file(filename.c_str());
	addShader(fragmentLighting, shaderData, GL_FRAGMENT_SHADER_ARB, true);

	// default to using vertex lighting program
	curProgram = vertexLighting;

	// import external geometry
	filename = media.get_file("models/Rayguns/Raygun_01.OBJ");
	if (filename == "")
	{
	printf("Unable to locate Raygun_01.OBJ, exiting...\n");

	}

	/*  if (!LoadObjModel(filename.c_str(),nverts,nindices,indices,vertexdata,normaldata,tangendata,binormdata,texcoords))
	{
	printf("Unable to load Raygun_01.OBJ, exiting...\n");

	}*/

}
void GLArea ::addShader(GLhandleARB programObject, const GLcharARB *shaderSource, GLenum shaderType, bool linkProgram)
{
	assert(programObject != 0);
	assert(shaderSource != 0);
	assert(shaderType != 0);

	GLhandleARB object = glCreateShaderObjectARB(shaderType);
	assert(object != 0);

	GLint length = (GLint)strlen(shaderSource);
	glShaderSourceARB(object, 1, &shaderSource, &length);

	// compile vertex shader object
	glCompileShaderARB(object);

	// check if shader compiled
	GLint compiled = 0;
	glGetObjectParameterivARB(object, GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

	if (!compiled)
	{

	}

	// attach vertex shader to program object
	glAttachObjectARB(programObject, object);

	// delete vertex object, no longer needed
	glDeleteObjectARB(object);

	if (linkProgram)
	{
		glLinkProgramARB(programObject);

		GLint linked = false;
		glGetObjectParameterivARB(programObject, GL_OBJECT_LINK_STATUS_ARB, &linked);
		if (!linked)
		{

			cout << "Shaders failed to link, exiting..." << endl;
		}
	}

	GLenum err = glGetError();
	if (err != GL_NO_ERROR)
		cout << "OpenGL error: " << gluErrorString(err) << endl;
}