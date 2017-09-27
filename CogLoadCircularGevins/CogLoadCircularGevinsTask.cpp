////////////////////////////////////////////////////////////////////////////////
// $Id: $
// Authors: Kedar S. Prabhudesai (for SSPACISS at Duke University)
// Description: CogLoadCircularGevinsTask implementation
//   
//   
// $BEGIN_BCI2000_LICENSE$
// 
// This file is part of BCI2000, a platform for real-time bio-signal research.
// [ Copyright (C) 2000-2012: BCI2000 team and many external contributors ]
// 
// BCI2000 is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// BCI2000 is distributed in the hope that it will be useful, but
//                         WITHOUT ANY WARRANTY
// - without even the implied warranty of MERCHANTABILITY or FITNESS FOR
// A PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program.  If not, see <http://www.gnu.org/licenses/>.
// 
// $END_BCI2000_LICENSE$
////////////////////////////////////////////////////////////////////////////////
#include "PCHIncludes.h"
#pragma hdrstop

#include "CogLoadCircularGevinsTask.h"
#include "BCIError.h"


using namespace std;


RegisterFilter( CogLoadCircularGevinsTask, 3 );
// Change the location if appropriate, to determine where your filter gets
// sorted into the chain. By convention:
//  - filter locations within SignalSource modules begin with "1."
//  - filter locations within SignalProcessing modules begin with "2."  
//       (NB: SignalProcessing modules must specify this with a Filter() command in their PipeDefinition.cpp file too)
//  - filter locations within Application modules begin with "3."


CogLoadCircularGevinsTask::CogLoadCircularGevinsTask()
{

	// Declare any parameters that the filter needs....

	BEGIN_PARAMETER_DEFINITIONS

		"Application:CogLoadCircularGevinsTask int taskType= 0 0 0 4 // Select Task Type: 0 %20 1 Verbal%20n%20Back, 2 Verbal%20First%20Back 3 Spatial%20n%20Back 4 Spatial%20First%20Back (enumeration)",

		END_PARAMETER_DEFINITIONS


		// ...and likewise any state variables:

		BEGIN_STATE_DEFINITIONS

		"TaskDispMode       8 0 0 3",  
		"myScore			   8 0 0 255",

		END_STATE_DEFINITIONS

}


CogLoadCircularGevinsTask::~CogLoadCircularGevinsTask()
{
	Halt();
}

void
	CogLoadCircularGevinsTask::Halt()
{
	// De-allocate any memory reserved in Initialize, stop any threads, etc.
	// Good practice is to write the Halt() method such that it is safe to call it even *before*
	// Initialize, and safe to call it twice (e.g. make sure you do not delete [] pointers that
	// have already been deleted:  set them to NULL after deletion).
	delete mTaskObj;
}

void
	CogLoadCircularGevinsTask::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
{
	// The user has pressed "Set Config" and we need to sanity-check everything.
	// For example, check that all necessary parameters and states are accessible:
	//
	// Parameter( "Milk" );
	// State( "Bananas" );
	//
	// Also check that the values of any parameters are sane:
	//
	// if( (float)Parameter( "Denominator" ) == 0.0f )
	//      bcierr << "Denominator cannot be zero" << endl;
	// 
	// Errors issued in this way, during Preflight, still allow the user to open
	// the Config dialog box, fix bad parameters and re-try.  By contrast, errors
	// and C++ exceptions at any other stage (outside Preflight) will make the
	// system stop, such that BCI2000 will need to be relaunched entirely.

	// Note that the CogLoadCircularGevinsTask instance itself, and its members, are read-only during
	// this phase, due to the "const" at the end of the Preflight prototype above.
	// Any methods called by Preflight must also be "const" in the same way.
	PreflightCondition(Parameter("taskType") > 0);
	Output = Input; // this simply passes information through about SampleBlock dimensions, etc....

}


void
	CogLoadCircularGevinsTask::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
	// The user has pressed "Set Config" and all Preflight checks have passed.
	// The signal properties can no longer be modified, but the const limitation has gone, so
	// the CogLoadCircularGevinsTask instance itself can be modified. Allocate any memory you need, start any
	// threads, store any information you need in private member variables.
	mSoundFileName = "C:\\BCI2000_ksp6\\prog\\sounds\\result.wav";
	mIsSoundPlayed = false;

	mTaskObj = new (nothrow) nBackTestCircGevins;
	PreflightCondition(mTaskObj != 0);

	mTaskType = Parameter("taskType");

	mTaskObj->showMaximized();
	if(!mTaskObj->isReady) mTaskObj->InitTask(mTaskType);
	mTaskObj->show();
}

void
	CogLoadCircularGevinsTask::StartRun()
{
	// The user has just pressed "Start" (or "Resume")
	bciout << "Hello World!" << endl;
	mTaskObj->StartTask();
}


void
	CogLoadCircularGevinsTask::Process( const GenericSignal& Input, GenericSignal& Output )
{

	// And now we're processing a single SampleBlock of data.
	// Remember not to take too much CPU time here, or you will break the real-time constraint.
	/*mTaskObj->ProcessTask();
	State("TaskDispMode") = mTaskObj->taskDispMode;
	State("myScore") = mTaskObj->myScore;
*/
	if(!mTaskObj->isDone)
	{
		mTaskObj->ProcessTask();
		State("TaskDispMode") = mTaskObj->taskDispMode;
		State("myScore") = mTaskObj->myScore;
	}
	else
	{
		if(!mIsSoundPlayed)
		{
			mMediaObject.SetFile(mSoundFileName);
			mMediaObject.Play();
			mIsSoundPlayed = true;
		}
	}
	Output = Input; // This passes the signal through unmodified.                  
}

void
	CogLoadCircularGevinsTask::StopRun()
{
	// The Running state has been set to 0, either because the user has pressed "Suspend",
	// or because the run has reached its natural end.
	bciout << "Goodbye World." << endl;
	// You know, you can delete methods if you're not using them.
	// Remove the corresponding declaration from CogLoadCircularGevinsTask.h too, if so.
}
