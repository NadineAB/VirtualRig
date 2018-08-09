#ifndef  _BVH_H_
#define  _BVH_H_
#include <vector>
#include <map>
#include <string>
#include <vcg/math/matrix33.h>
#include <vcg/math/matrix44.h>
#include "BVH.h"
using namespace vcg; 
using namespace  std; 
/*! 
* \brief:
* Base class used to import motion capture file format, providing interface 
* to work with skeleton hierarchy information as well as motion data
* coded by Marco Fratarcangeli
*/
class  BVH  // BVH : Biovision Hierarchy (BVH) character animation file format 
{
	/*!  BVH containts of : 
	* Joint
	* Channel/DOF 
	* Motion
	* Frame
	*/
public:
	// Enumeration which have XYZ for Rotation (3DOF) 
	//and Translate which have XYZ  (3DOF)
	enum  ChannelEnum
	{
		X_ROTATION, Y_ROTATION, Z_ROTATION,  
		X_POSITION, Y_POSITION, Z_POSITION   
	};// in total (6DOF)

	struct  Joint; // define Joint below

	struct  Channel
	{
		Joint *              joint;  // pointer of type joint
		ChannelEnum          type;   // which type of channel (it could be rotate or translate in specific axis)     
		int                  index;  // index of channel
	};

	struct  Joint
	{
		string               name; // name of the joint ( ex : head / left shoulder / root)
		int                  index; // index of the joint which important becouse we ganna use it in channel to indicate speacific degree of freedom 
		Joint *              parent; // pointer for parent (one parent)
		vector< Joint * >    children; // pointer of the children
		double               offset[3];  
		// the translation the bone with respact to
		// the parent  (b)O========O(P)  it could be useful to defining the lenght/ direction of the parent's bone 
		bool                 has_site;   // the end effector
		double               site[3];    // the offset of the end effector joint

		Matrix33<float> L;	      // local coordinate frame of the bone
		Matrix33<float> mrot;	 // rotation from the previous frame to the current one
		Point3f			wc;	 // position in world coordinates ( model coordinates)

		vector< Channel * >  channels; 
		// vector pointer of type channel which could be max 6DOF
		// which encapsulate Joint object , index of the joint and channel type
	};


private:

	bool                     is_load_success; // flag to know if it load successfully or not

	string                   file_name;    // file name
	string                   motion_name;  // motion name

	int                      num_channel;  // where motion cantian also channel data
	vector< Channel * >      channels;    // vector pointer of type channel
	vector< Joint * >        joints;      // vector pointer of joint

	map< string, Joint * >   joint_index; // map contain the joint name 
	// and pointer of type joint 

	int                      num_frame;   // number of the frames
	double                   interval;    // long of the motion
	double *                 motion;      // animation frame
	// where for each frame channel data for each bone is defined
	// which is channel data for bone as it define in hierarchy

public:
	BVH(); // empty constr 
	BVH( const char * bvh_file_name ); // constr take file name as paramter
	~BVH(); // destr

	void  Clear(); // clear the BVH file

	void  Load( const char * bvh_file_name ); // load BVH file 

public:
	bool  IsLoadSuccess() const { return is_load_success; } 
	// flag to know if its success or not

	const string &  GetFileName() const { return file_name; }
	// return the file name

	const string &  GetMotionName() const { return motion_name; }
	// return the motion name

	const int       GetNumJoint() const { return  joints.size(); }
	// return size of joints , number of joints in the skeleton

	const Joint *   GetJoint( int no ) const { return  joints[no]; }
	// retrive the joint by the index

	const int       GetNumChannel() const { return  channels.size(); }
	// return the channel number of the joint the { DOF for each joint }

	const Channel * GetChannel( int no ) const { return  channels[no]; }
	// get the specific channel by the channel index { for translate X/Y/Z or Rotate X/Y/Z 

	const Joint *   GetJoint( const string & j ) const  {
		map< string, Joint * >::const_iterator  i = joint_index.find( j );
		return  ( i != joint_index.end() ) ? (*i).second : NULL; }
	// given specific joint name as string search and return it as joint 

	const Joint *   GetJoint( const char * j ) const  {
		map< string, Joint * >::const_iterator  i = joint_index.find( j );
		return  ( i != joint_index.end() ) ? (*i).second : NULL; }
	// given specific joint name as character search and return it as joint 

	int     GetNumFrame() const { return  num_frame; }
	// return the number of frames

	double  GetInterval() const { return  interval; }
	// return the duration of the animation

	double  GetMotion( int f, int c ) const { return  motion[ f*num_channel + c ]; }
	// return motion value given frame num and channel index 

	void  SetMotion( int f, int c, double v ) { motion[ f*num_channel + c ] = v; }
	// set specific type value for the motion
public:
	// Render methods
	void  RenderFigure( int frame_no ); // render frame
	static void  RenderFigure( Joint * root, const double * data ); // render the whole skeleton 
	static void  RenderBone( Joint * joint, float x0, float y0, float z0, float x1, float y1, float z1 ); // render bone by bone
	// XYZ Scale and Translate
	static float scalex;    
	static float scaley;
	static float scalez;
	static float transx;
	static float transy;
	static float transz;
};
#endif // _BVH_H_