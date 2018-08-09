#include <fstream>
#include <string.h>
#include <math.h>
#include <GL/glew.h>
#include "BVH.h"
/////// define profile and profile node to estimate code proformance 
#include "profile.h"
// define the size and postion of the skeleton
// in the work space ( inital pose)
// 8
// octupas
float BVH::scalex = 1.0f;
float BVH::scaley = 1.0f;
float BVH::scalez = 1.0f;

float BVH::transx = 0;//-5.9f;
float BVH::transy = 0;//-10.7f;
float BVH::transz = 0;//11.8f;

BVH::BVH()
{
	motion = NULL;  // define the double veriable of motion equal to null 
	Clear();  // call clear method
}


BVH::BVH( const char * bvh_file_name )
{
	motion = NULL;
	Clear();

	Load( bvh_file_name ); // call load method and
	// send the file name as pointer of char or string
}


BVH::~BVH()
{
	Clear();  // call clear method
}

void  BVH::Clear()
{
	// delete any dynamic allocation 
	int  i;
	for ( i=0; i<channels.size(); i++ )
		delete  channels[ i ];
	for ( i=0; i<joints.size(); i++ )
		delete  joints[ i ];
	if ( motion != NULL )
		delete  motion;

	is_load_success = false; // set load BVH falg to flase
	// empty file name and motion name
	// delete map joint index veriable
	// and re initialize any other veriable 
	file_name = "";
	motion_name = "";
	num_channel = 0;
	channels.clear();
	joints.clear();
	joint_index.clear();  
	num_frame = 0;
	interval = 0.0;
	motion = NULL;
}

/*! \brief: load BVH file
* 
* \param bvh_file_name		path and file name on the disk
*
*/
void  BVH::Load( const char * bvh_file_name )
{
	
     #define  BUFFER_LENGTH  1024*10

	ifstream  file;
	char      line[ BUFFER_LENGTH ];
	char *    token;
	char      separater[] = " :,\t";
	// local veriable of type joint
	vector< Joint * >   joint_stack;
	Joint *   joint = NULL;
	Joint *   new_joint = NULL;
	bool      is_site = false;
	double    x, y ,z;
	int       i, j;

	Clear();

	file_name = bvh_file_name;
	const char *  mn_first = bvh_file_name;
	const char *  mn_last = bvh_file_name + strlen( bvh_file_name );
	if ( strrchr( bvh_file_name, '\\' ) != NULL )
		mn_first = strrchr( bvh_file_name, '\\' ) + 1;
	else if ( strrchr( bvh_file_name, '/' ) != NULL )
		mn_first = strrchr( bvh_file_name, '/' ) + 1;
	if ( strrchr( bvh_file_name, '.' ) != NULL )
		mn_last = strrchr( bvh_file_name, '.' );
	if ( mn_last < mn_first )
		mn_last = bvh_file_name + strlen( bvh_file_name );
	motion_name.assign( mn_first, mn_last );


	file.open( bvh_file_name, ios::in );
	if ( file.is_open() == 0 )  return; // if you cant open the 
	// file return 0
	// loop till the end of the file
	while ( ! file.eof() )
	{

		if ( file.eof() )  goto bvh_error; // if we are at the end of the file
		// return error

		// read line by line 
		file.getline( line, BUFFER_LENGTH );
		token = strtok( line, separater );
		// where joint label by joint and root

		if ( token == NULL )  continue;
		// where each { consider as new joint 
		if ( strcmp( token, "{" ) == 0 )
		{
			// add joint to stack
			joint_stack.push_back( joint );
			joint = new_joint;
			continue;
		}
		// end of the joint
		if ( strcmp( token, "}" ) == 0 )
		{

			joint = joint_stack.back();
			joint_stack.pop_back();
			is_site = false;
			continue;
		}
		// the joint its it first Root the very first joint or normal joint

		if ( ( strcmp( token, "ROOT" ) == 0 ) ||
			( strcmp( token, "JOINT" ) == 0 ) )
		{
			// joint might be ROOT or JOINT
			// where each Joint have number of joints
			new_joint = new Joint();
			new_joint->index = joints.size();
			new_joint->parent = joint;
			new_joint->has_site = false;
			new_joint->offset[0] = 0.0;  new_joint->offset[1] = 0.0;  new_joint->offset[2] = 0.0;
			new_joint->wc[0] = 0.0;  new_joint->wc[1] = 0.0;  new_joint->wc[2] = 0.0;
			new_joint->site[0] = 0.0;  new_joint->site[1] = 0.0;  new_joint->site[2] = 0.0;
			joints.push_back( new_joint );
			if ( joint )
				joint->children.push_back( new_joint );

			token = strtok( NULL, "" );
			while ( *token == ' ' )  token ++;
			new_joint->name = token;


			joint_index[ new_joint->name ] = new_joint;
			continue;
		}


		if ( ( strcmp( token, "End" ) == 0 ) )
		{
			new_joint = joint;
			is_site = true;
			continue;
		}
		// postion of the joint
		if ( strcmp( token, "OFFSET" ) == 0 )
		{

			token = strtok( NULL, separater );
			x = token ? atof( token ) : 0.0;
			token = strtok( NULL, separater );
			y = token ? atof( token ) : 0.0;
			token = strtok( NULL, separater );
			z = token ? atof( token ) : 0.0;

			// end effector of the joints chain 
			if ( is_site )
			{
				joint->has_site = true;
				joint->site[0] = x;
				joint->site[1] = y;
				joint->site[2] = z;
			}
			else
				// if its not, this is not the end of chain
			{
				joint->offset[0] = x;
				joint->offset[1] = y;
				joint->offset[2] = z;

				joint->wc[0] = x;
				joint->wc[1] = y;
				joint->wc[2] = z;
			}
			continue;
		}

		// which describe the degree of freedom
		if ( strcmp( token, "CHANNELS" ) == 0 )
		{
			// each channel as keyboard follow by number and 
			// type of transformation
			token = strtok( NULL, separater );
			joint->channels.resize( token ? atoi( token ) : 0 );

			// add channel
			for ( i=0; i<joint->channels.size(); i++ )
			{

				Channel *  channel = new Channel();
				channel->joint = joint;
				channel->index = channels.size();
				channels.push_back( channel );
				joint->channels[ i ] = channel;


				token = strtok( NULL, separater );
				if ( strcmp( token, "Xrotation" ) == 0 )
					channel->type = X_ROTATION;
				else if ( strcmp( token, "Yrotation" ) == 0 )
					channel->type = Y_ROTATION;
				else if ( strcmp( token, "Zrotation" ) == 0 )
					channel->type = Z_ROTATION;
				else if ( strcmp( token, "Xposition" ) == 0 )
					channel->type = X_POSITION;
				else if ( strcmp( token, "Yposition" ) == 0 )
					channel->type = Y_POSITION;
				else if ( strcmp( token, "Zposition" ) == 0 )
					channel->type = Z_POSITION;
			}
		}

		// if the keyword is motin break !!!!
		if ( strcmp( token, "MOTION" ) == 0 )
			break;
	}
	// get which frame we are now	
	file.getline( line, BUFFER_LENGTH );
	token = strtok( line, separater );
	if ( strcmp( token, "Frames" ) != 0 )  goto bvh_error;
	token = strtok( NULL, separater );
	if ( token == NULL )  goto bvh_error;
	num_frame = atoi( token );
	// number of frames
	file.getline( line, BUFFER_LENGTH );
	token = strtok( line, ":" );
	if ( strcmp( token, "Frame Time" ) != 0 )  goto bvh_error;
	token = strtok( NULL, separater );
	if ( token == NULL )  goto bvh_error;
	interval = atof( token );

	num_channel = channels.size();
	motion = new double[ num_frame * num_channel ]; // create arrary with number of frame and number of channel 

	// then start to read frame by frame
	for ( i=0; i<num_frame; i++ )
	{
		file.getline( line, BUFFER_LENGTH );
		token = strtok( line, separater );
		for ( j=0; j<num_channel; j++ )
		{
			if ( token == NULL )
				goto bvh_error;
			motion[ i*num_channel + j ] = atof( token );
			token = strtok( NULL, separater );
		}
	}
	// close 
	file.close();
	// set flag to one
	is_load_success = true;
	
	return;
     bvh_error:
     file.close();


}
/*! \brief:  Render method frame by frame
* 
* \param frame_no		Frame number
*
*/
void  BVH::RenderFigure( int frame_no )
{
	glPushMatrix();
	glLoadIdentity(	);
	RenderFigure( joints[ 0 ], motion + frame_no * num_channel );  
	// call RenderFigure and send the root and motion data
	glPopMatrix();
}
/*! \brief:  Render method the frame component
* 
* \param Joint		which recive the first joint in the hierarchy
* \param frame_no		motin with respect to each joint
*/
void  BVH::RenderFigure( Joint * joint, const double * data )
{
	glPushMatrix();
	// it just the root joint boz no parent mainly just the root or hip can be translated 
	// all joint have to rotate
	if ( joint->parent == NULL )
	{
		// move the root, that is the full skeleton
		glTranslatef( data[ 0 ] * scalex, data[ 1 ] * scaley, data[ 2 ] * scalez );
	}
	// 
	else
	{
		// move the joint in the pivot point (initially it is in the same position of the parent)
		glTranslatef( joint->offset[ 0 ] * scalex, joint->offset[ 1 ] * scaley, joint->offset[ 2 ] * scalez );
	}

	int  i;
	// rotate the joint around first x,then y and then z axis respectively
	for ( i=0; i<joint->channels.size(); i++ )
	{
		Channel * channel = joint->channels[ i ];
		double rot = data[channel->index];
		if (channel->type == X_ROTATION)
			glRotatef(rot, 1.0f, 0.0f, 0.0f);
		else if (channel->type == Y_ROTATION)
			glRotatef(rot, 0.0f, 1.0f, 0.0f);
		else if (channel->type == Z_ROTATION)
			glRotatef(rot, 0.0f, 0.0f, 1.0f);
	}
	// render each childern bone by render bone method
	// if it equal zero that mean, this end effector
	if ( joint->children.size() == 0 )
	{
		RenderBone( joint, 0.0f, 0.0f, 0.0f, joint->site[ 0 ] * scalex, joint->site[ 1 ] * scaley, joint->site[ 2 ] * scalez );
	}
	// one child
	if ( joint->children.size() == 1 )
	{
		Joint *  child = joint->children[ 0 ];
		RenderBone( joint, 0.0f, 0.0f, 0.0f, child->offset[ 0 ] * scalex, child->offset[ 1 ] * scaley, child->offset[ 2 ] * scalez );
	}
	// more than one
	if ( joint->children.size() > 1 )
	{
		float  center[ 3 ] = { 0.0f, 0.0f, 0.0f };
		for ( i=0; i<joint->children.size(); i++ )
		{
			Joint *  child = joint->children[ i ];
			center[ 0 ] += child->offset[ 0 ];
			center[ 1 ] += child->offset[ 1 ];
			center[ 2 ] += child->offset[ 2 ];
		}

		center[ 0 ] /= joint->children.size() + 1;
		center[ 1 ] /= joint->children.size() + 1;
		center[ 2 ] /= joint->children.size() + 1;

		RenderBone(	joint, 0.0f, 0.0f, 0.0f, center[ 0 ] * scalex, center[ 1 ] * scaley, center[ 2 ] * scalez );

		for ( i=0; i<joint->children.size(); i++ )
		{
			Joint *  child = joint->children[ i ];
			RenderBone(	joint, 
				center[ 0 ] * scalex, 
				center[ 1 ] * scaley, 
				center[ 2 ] * scalez,
				child->offset[ 0 ] * scalex, 
				child->offset[ 1 ] * scaley, 
				child->offset[ 2 ] * scalez );
		}
	}

	// go down in the hierarchy (depth first)
	for ( i=0; i < joint->children.size(); i++ )
	{
		RenderFigure( joint->children[ i ], data);
	}

	glPopMatrix();
}

/*! \brief:  Render method the frame component
* 
* \param Joint		which recive the first joint in the hierarchy
* \scale param      	
* \ in order to compute the bone lenght which can be compute from distance 
* \ of origin joint
*/
void  BVH::RenderBone( Joint * joint, float x0, float y0, float z0, float x1, float y1, float z1 )
{
	GLdouble  dir_x = x1 - x0;
	GLdouble  dir_y = y1 - y0;
	GLdouble  dir_z = z1 - z0;
	GLdouble  bone_length = sqrt( dir_x*dir_x + dir_y*dir_y + dir_z*dir_z );

	glPushMatrix();

	glTranslated( x0, y0, z0 );

	double  length;
	length = sqrt( dir_x*dir_x + dir_y*dir_y + dir_z*dir_z );
	if ( length < 0.0001 ) { 
		dir_x = 0.0; dir_y = 0.0; dir_z = 1.0;  length = 1.0;
	}
	dir_x /= length;  dir_y /= length;  dir_z /= length;  // normlize the bone direction

	GLdouble  up_x, up_y, up_z;
	up_x = 0.0;
	up_y = 1.0;
	up_z = 0.0;
	/*
	-----------------------------------------------------------------
	establish the matrix that convert the postion in local coordinate 
	to world coordinate 
	-----------------------------------------------------------------
	*/
	double  side_x, side_y, side_z;
	side_x = up_y * dir_z - up_z * dir_y;
	side_y = up_z * dir_x - up_x * dir_z;
	side_z = up_x * dir_y - up_y * dir_x;

	length = sqrt( side_x*side_x + side_y*side_y + side_z*side_z );
	if ( length < 0.0001 ) {
		side_x = 1.0; side_y = 0.0; side_z = 0.0;  length = 1.0;
	}
	side_x /= length;  side_y /= length;  side_z /= length;

	up_x = dir_y * side_z - dir_z * side_y;
	up_y = dir_z * side_x - dir_x * side_z;
	up_z = dir_x * side_y - dir_y * side_x;

	// construct matrix transform from bone to world
	GLdouble  m[16] = { side_x, side_y, side_z, 0.0,
		up_x,   up_y,   up_z,   0.0,
		dir_x,  dir_y,  dir_z,  0.0,
		0.0,    0.0,    0.0,    1.0 };
	glMultMatrixd( m );

	GLdouble mm[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, mm);

	Matrix44d mv(mm);
	Transpose(mv);

	Point4d pov = mv * Point4d(0, 0, 0, 1);
	joint->wc = Point3f(pov[0] + transx, pov[1] + transy, pov[2] + transz);

	Point3f z_prev = joint->L.GetColumn(2); // local 

	joint->L.SetColumn(0, Point3f(mm[0], mm[1], mm[2]));
	joint->L.SetColumn(1, Point3f(mm[4], mm[5], mm[6]));
	joint->L.SetColumn(2, Point3f(mm[8], mm[9], mm[10]));

	Point3f z_curr = joint->L.GetColumn(2);

	float angle = Angle(z_prev, z_curr);
	Point3f axis = (z_prev ^ z_curr).Normalize();

	joint->mrot.SetRotateRad(angle, axis);


	//glDisable(GL_LIGHTING);
	//glBegin(GL_LINES);
	//glColor3f(1, 1, 1);
	//glVertex3f(0, 0, 0);
	//glVertex3f(0, 0, bone_length);
	//glColor3f(1, 0, 0);
	//glVertex3f(0, 0, 0);
	//glVertex3f(side_x, side_y, side_z );
	//glColor3f(0, 1, 0);
	//glVertex3f(0, 0, 0);
	//glVertex3f(up_x, up_y, up_z );
	//glColor3f(0, 0, 1);
	//glVertex3f(0, 0, 0);
	//glVertex3f(dir_x , dir_y , dir_z );
	//glEnd();
	//glEnable(GL_LIGHTING);

	glPopMatrix();
}

// End of BVH.cpp
