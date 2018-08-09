/*
* @brief   Profile system
*
*****************************************************************************
*
* Based from article on Gems Game programming 3
* Original authors GregHjelstrom and Byon Garrabrant
*
*/

#ifndef _IDO_PROFILE_H
#define _IDO_PROFILE_H

#ifdef USE_PROFILE

#pragma warning(disable : 4996)

#include <vector>
#include <cstring>
#include <ostream>
#include "profile_node.h"

/**
*  Main class for the profile system. Example of how to use the profile system
*  imagine you have a function :
*
*  int Test()
*  {
*
*    TestA();
*    TestB();
*    TestC();
*
*  }
*
*  if you want to profile the entire function you should do it like that:
*
*  int Test()
*  {
*    PRF_SAMPLE("Sample Test");
*
*    TestA();
*    TestB();
*    TestC();
*
*  }
*
*
*  Imagine now you want to sample only a part of Test like only TestA and TestB
*
*  int Test()
*  {
*
*    { 
*     PRF_SAMPLE("TestA and TestB");
*     TestA();
*     TestB();
*    }
*
*    TestC();
*
*  }
*
*  So with these example you can sample all functions you want
*/

class Profile
{

	/// root node
	ProfileNode* mRoot;

	/// current node
	ProfileNode* mCurrentNode;

	/// level
	int mLevel;

public:


	/// Default constructor
	Profile()
	{ 
		mRoot=new ProfileNode("PROFILE_ROOT"); 
		mCurrentNode=mRoot;
		mLevel=0; 
	}


	~Profile()
	{ 
		delete mRoot;
	}



	/** 
	*  Return Root node
	*
	*  @return ProfileNode* root node
	*/
	ProfileNode* GetRootNode() { return mRoot;}



	/** 
	*  Start profile. It's create the node the first time, after it update the profile
	*
	*  @param const char* name of node to create or to update
	*/
	void StartProfile(const char* _name)
	{ 
		ProfileNode *node;

		// recursif function
		if (mCurrentNode->GetName()==_name) {
			mCurrentNode->AddCalls();
			mLevel++;
			return;
		}

		node=mCurrentNode->CheckSubName(_name) ;
		if (!node) {
			node=new ProfileNode(_name);
			mCurrentNode->AddNode(node);
		}

		node->StartProfile();
		mCurrentNode=node;
		mLevel++;
	}




	/** 
	*  Stop the profile
	*/
	void StopProfile()
	{
		mLevel--;

		// it is a recursif function ? if no we stop profile
		if (mCurrentNode->GetLevel() == mLevel+1) {
			mCurrentNode->StopProfile();


			if (mCurrentNode->GetParent())
				mCurrentNode=mCurrentNode->GetParent();
		}
	}




	/** 
	*  Set the limit subnode to examine. It's call Filter of the root node
	*
	*  @param int the limit depth
	*/
	void SetFilter(int _level) { mRoot->SetFilter(_level);}



	/** 
	*  Update total time
	*
	*  @param double use a global time to compute good percent ...
	*/
	void SetGlobalTime(double _globalTime) { mRoot->SetGlobalTime(_globalTime);}

	/**
	* Update number of frames
	*/
	void EndFrame()
	{
		GetRootNode()->mReferenceCount++;
	}

	/**
	* Reset
	*/
	void Reset()
	{
		// reset static members
		GetRootNode()->mReferenceCount = 0;
		GetRootNode()->mGlobalTime = 0;

		// reset recursively each node
		GetRootNode()->Reset();
	}

};

/**
*  Return the profiler (singleton)
*/
Profile* profile=0;
Profile* GetProfiler() 
{
	if (!profile)
		profile=new Profile;
	return profile;
}

void DeallocateProfiler()
{
	if (profile)
		delete profile;
}


/**
*  This class is used to make a sample, that will be updated each time this sample 
*  is executed  by your program. See the macro PRF_SAMPLE to make sample
*/
class Sample
{

public:

	/** First entry to the profile sample*/
	Sample(const char *name)
	{ 
		GetProfiler()->StartProfile(name); 
	}

	/** Stop the Profile sample*/
	~Sample()
	{ 
		GetProfiler()->StopProfile(); 
	}

};


/** 
*  Use this macro to profile a function
*/
#define PROFILE_SAMPLE(a) Sample P__LINE__(a)

#else

#define PROFILE_SAMPLE(a)

#endif

#endif

