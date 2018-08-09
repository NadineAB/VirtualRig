/*
* @brief   Profile system
*
*****************************************************************************
*
* Based from article on Gems Game programming 3
* Original authors GregHjelstrom and Byon Garrabrant
*
*/

#ifndef _IDO_PROFILE_NODE_H
#define _IDO_PROFILE_NODE_H

#include <vector>
#include <ostream>

#include "timer.h"


/** 
*  This class is used as node in a array tree
*/
class ProfileNode : public std::vector<ProfileNode*>
{

public:

	typedef std::vector<ProfileNode*> ContainerNode;

private:

	/// start time
	double mStartTime;

	/// end time
	double mEndTime;

	/// time of function
	double mDuringTime;

	/// total time of function
	double mTotalTime;

	/// number of call
	int mNbCalls;

	/// level of the current node, we can see if a function is recursif with it
	int mLevel;

	/// specific data to display informations
	/*static*/ int mMaxSubSizeName;

	/// Frame counter
	unsigned int mCount;


	/// depth of profile
	static int mFilter;


	/// Name of node
	const char* mName;

	/// Parent node
	ProfileNode* mParent;


public:

	/// Total time reference
	static double mGlobalTime;

	/// Reference frame counter
	static unsigned int mReferenceCount;

	/// Default constructor
	ProfileNode(const char* _name) {
		mName=_name; 
		Init();
		mTotalTime=mStartTime=mDuringTime=0;
		mCount=0;
		mNbCalls=0;
		mParent=0;
		mLevel=0;
		reserve(10);
		mMaxSubSizeName = 8;
	}



	/**
	*  Default destructor
	*/
	~ProfileNode() {
		while (!empty()) {
			delete back();
			pop_back();
		} 
	}


	inline double GetTotalTime() { return mTotalTime;}
	inline double GetGlobalTime() { return mGlobalTime;}
	inline int GetNbCalls() { return mNbCalls;}
	inline int GetSubSizeName() { return mMaxSubSizeName;}
	inline int GetLevel() { return mLevel;}

	/** 
	*  Set parent to this node
	*
	*  @param ProfileNode* parent to affect
	*/
	void SetParent(ProfileNode* _parent) { mParent=_parent;}



	/** 
	*  Get Parent of this node
	*
	*  @return ProfileNode* parent
	*/
	ProfileNode* GetParent() { return mParent;}


	/** 
	*  Add a child node of this node
	*
	*  @param ProfileNode* not to add
	*/
	void AddNode(ProfileNode* _node) {
		int strl=(int)strlen(_node->GetName());
		if (mMaxSubSizeName<strl)
			mMaxSubSizeName=strl;
		_node->SetParent(this); 
		push_back(_node);
		_node->SetLevel(GetLevel()+1);
	}




	/** 
	*  Return the name of node
	*
	*  @param const char* name of node
	*/
	const char* GetName() const { return mName;}


	/** 
	*  Check if a node with the given name exist
	*
	*  @param const char* name of node to check
	*  @return ProfileNode* node if found else 0
	*/
	ProfileNode* CheckSubName(const char* _name) { 
		ContainerNode::iterator it=begin();
		for (;it!=end();it++) 
			if ( (*it)->GetName() == _name)
				return *it;
		return 0;
	}



	/**
	*  Get the level of this node
	*
	*  @return int the level of node
	*/
	int GetLevel() const { return mLevel;}



	/**
	*  Set level of this node
	*
	*  @param int level to affect
	*/
	void SetLevel(int _level) { mLevel=_level;}



	/// Incremente number of call
	void AddCalls() { mNbCalls+=1;}

	/// Init
	void Init()
	{
		mTotalTime=mStartTime=mDuringTime=mGlobalTime=0;
		mCount=mReferenceCount=0;
		mNbCalls=0;
	}

	/// Reset
	/// reset the data of the profile node
	/// note that also mReferenceCount and mGlobalTime must be reset to zero
	void Reset()
	{
		mTotalTime = 0;
		mNbCalls = 0;

		// recursively call each children node and reset
		ContainerNode::iterator n=begin();
		for (;n!=end();n++) 
			(*n)->Reset();
	}

	/// Start profile
	void StartProfile() { 

		if (mFilter && mFilter<mLevel)
			return;

		if (mReferenceCount!=mCount) {
			mCount=mReferenceCount;

			//			mTotalTime=0;
			mNbCalls=0;
		}

		mStartTime=GetRealTime(); 
	}



	/// Stop profile
	void StopProfile() { 
		if (mFilter && mFilter<mLevel)
			return ;

		AddCalls(); 
		mDuringTime=GetRealTime(); 
		mDuringTime-=mStartTime; 
		mTotalTime+=mDuringTime;

		if (mLevel==1)
			mGlobalTime+=mDuringTime;
	}


	/** 
	*  Set filter. It affect all node
	*
	*  @param int filter depth
	*/
	void SetFilter(int _depth) { mFilter=_depth;}



	/** 
	*  New frame restart profiling for next frame
	*
	*  @param double global time to use
	*/
	void SetGlobalTime(double _globalTime) {
		mGlobalTime=_globalTime;
		mReferenceCount++;
	}

	void DisplayFlatItem(std::ostream& _out) {
		double ptime=GetTotalTime();
		double totalTime=ptime;
		double globalTime=GetGlobalTime();
		int nbCalls=GetNbCalls();
		ProfileNode* parent=GetParent();

		if (parent && parent->GetTotalTime()>0.)
			ptime=parent->GetTotalTime();

		if (ptime && globalTime && totalTime)
		{
			char str[256];
			char littleString[]=" ";
			sprintf(str,"%*s %-*s prt %6.2f glb %6.2f - time %.2f",
				(GetLevel()-1)*2, littleString,			// indent the node stats
				parent->GetSubSizeName(), GetName(),	// retrieve the node name
				totalTime * 100.0 / ptime,				// = percentage of time spent by this node within the time spent by the parent		
				totalTime * 100.0 / globalTime,			// = percentage of time spent by this node in the globalTime
				totalTime);								// total time spent by this node

			_out << str;

			if (nbCalls > 1)
			{
				sprintf(str, " avg %.4f - ncalls %-7d", 
					totalTime / nbCalls,					// total time spent by this node for each call
					nbCalls);								// number of calls

				_out << str;
			}


			if (GetLevel() == 1)	// nodes at highest level also shows the fps
			{
				sprintf(str, " - fps: %.2f avg fps: %.2f", 1 / mDuringTime, mReferenceCount / ptime);
				_out << str;
			}

			_out << "\n";
		}
	}


	void DisplayFlatUnloggedTime(std::ostream& _out,double subtime) {
		double ptime=GetTotalTime();
		double totalTime=ptime;
		double globalTime=GetGlobalTime();

		char littleString[]=" ";
		char str[128];
		sprintf(str,"%*s %-*s prt %6.2f glb %6.2f - time %.2f\n",
			GetLevel()*2, littleString,								// indent the node stats
			GetSubSizeName(),"Unlogged",
			subtime*100.0/totalTime,			
			subtime*100.0/globalTime,
			subtime);
		_out << str;
	}


	// display profiled data
	//
	//  example usage :
	//	std::stringstream op;
	//  GetProfiler()->GetRootNode()->DisplayFlatStats(op);
	//  Print(op);

	void DisplayFlatStats(std::ostream& _out) {

		if (GetLevel()>0)
			DisplayFlatItem(_out);	// display this node

		double subTime=0;

		// recursively call each children node and display flat stats
		ContainerNode::iterator n=begin();
		for (;n!=end();n++) {
			(*n)->DisplayFlatStats(_out);
			subTime+=(*n)->GetTotalTime();
		}

		// display time spent in not profiled activities
		if (subTime>0.) {
			subTime=GetTotalTime()-subTime;
			if (subTime>0.)
				DisplayFlatUnloggedTime(_out,subTime);
		}
	}

	/*
	void DisplayHeader(std::ostream& _out) {
	char header[128];
	sprintf(header," %-*s - %*s - %*s - %*s - %*s - %*s\n",
	10, GetName(),
	10,"Parent %%",
	10,"Global %%",
	10,"Time s",
	13,"Time/Call s",
	9,"nbCalls");
	_out << header;
	}

	void DisplayItemAsParent(std::ostream& _out) {
	double ptime=GetTotalTime();
	double totalTime=ptime;
	double globalTime=GetGlobalTime();
	int nbCalls=GetNbCalls();
	ProfileNode* parent=GetParent();
	if (parent && parent->GetTotalTime()>0.)
	ptime=parent->GetTotalTime();

	static char te[256] ;
	static char l[256];

	sprintf(te," %-*s - %9.2f - %9.2f - %10f - %13f - %9d <\n",
	GetSubSizeName(),GetName(),
	totalTime*100.0/ptime,totalTime*100.0/globalTime,
	totalTime,totalTime/nbCalls,
	nbCalls);

	_out << te;
	int t=(int)strlen(te);
	l[0]=' ';
	for (int i=1;i<t-3;i++)
	l[i]='-';
	l[t-3]='\n';
	l[t-2]=0;

	_out << l;
	}


	void DisplayChildItem(std::ostream& _out,const char *addBack) {
	double ptime=GetTotalTime();
	double totalTime=ptime;
	double globalTime=GetGlobalTime();
	int nbCalls=GetNbCalls();
	ProfileNode* parent=GetParent();
	if (parent && parent->GetTotalTime()>0.)
	ptime=parent->GetTotalTime();

	if (!addBack)
	addBack=" ";

	char ch[128];
	sprintf(ch," %-*s - %9.2f - %9.2f - %10f - %13f - %9d (%2d) %s\n",
	parent->GetSubSizeName(),GetName(),
	totalTime*100.0/ptime,totalTime*100.0/globalTime,
	totalTime,totalTime/nbCalls,
	nbCalls,size(),addBack);
	_out << ch;

	ContainerNode::iterator n=begin();
	for (;n!=end();n++) {
	(*n)->DisplayChildItem(_out, addBack);
	}
	}




	void DisplayUnloggedTime(std::ostream& _out,double subtime) {
	char ch[128];
	double globalTime=GetGlobalTime();
	double totalTime=GetTotalTime();
	sprintf(ch," %-*s - %9.2f - %9.2f - %10f\n",
	GetSubSizeName(),"Unlogged",
	subtime*100.0/totalTime,subtime*100.0/globalTime,
	subtime);
	_out << ch;
	}



	// addBack is the string to indent the childs
	void DisplayNode(std::ostream& _out,const char* addBack,ProfileNode* i) {

	DisplayHeader(_out);

	if (GetLevel()>0)
	DisplayItemAsParent(_out);

	double subTime=0;

	ContainerNode::iterator n=begin();
	for (;n!=end();n++) {
	(*n)->DisplayChildItem(_out, (*n==i?addBack:0) ) ;
	subTime+=(*n)->GetTotalTime();
	}

	if (subTime>0.) {
	subTime=GetTotalTime()-subTime;
	if (subTime>0.)
	DisplayUnloggedTime(_out,subTime);
	}
	}

	// hot to use the commented methods:
	//		ProfileNode* first = *(GetProfiler()->GetRootNode()->begin()++);
	//		GetProfiler()->GetRootNode()->DisplayNode(op, " ", first);
	//		first->DisplayNode(op, 0, GetProfiler()->GetRootNode());
	*/

};

/// Total time reference
double ProfileNode::mGlobalTime=0;

/// Reference frame counter
unsigned int ProfileNode::mReferenceCount=0;

/// depth of profile
int ProfileNode::mFilter=0;

/// specific data to display informations
//int ProfileNode::mMaxSubSizeName = 8;


#endif
