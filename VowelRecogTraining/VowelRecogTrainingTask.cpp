
////////////////////////////////////////////////////////////////////////////////
// $Id: $
// Authors: Kedar S. Prabhudesai (for SSPACISS at Duke University)
// Description: VowelRecogTrainingTask implementation
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

#include "VowelRecogTrainingTask.h"
#include "BCIError.h"

using namespace std;

RegisterFilter( VowelRecogTrainingTask, 3 );
// Change the location if appropriate, to determine where your filter gets
// sorted into the chain. By convention:
//  - filter locations within SignalSource modules begin with "1."
//  - filter locations within SignalProcessing modules begin with "2."  
//       (NB: SignalProcessing modules must specify this with a Filter() command in their PipeDefinition.cpp file too)
//  - filter locations within Application modules begin with "3."


VowelRecogTrainingTask::VowelRecogTrainingTask()
{
		// ...and likewise any state variables:
		// Remember isRight = 1: Right; 2: Wrong

		BEGIN_STATE_DEFINITIONS
		
		"TaskModes			 8 0 0 0 \r\n",
		"SpeakerIdx			 8 0 1 0 \r\n",
		"SpeakerSetIdx		 8 0 2 0 \r\n",
		"SpeakerSetOptionIdx 8 0 3 0 \r\n",
		"myScore			 8 0 4 0 \r\n",
		"isTaskDone			 1 0 5 0 \r\n",
		"isRight			 8 0 6 0 \r\n",
		"UserResponses		 8 0 7 0 \r\n",

		END_STATE_DEFINITIONS
}


VowelRecogTrainingTask::~VowelRecogTrainingTask()
{
	Halt();
}

void
	VowelRecogTrainingTask::Halt()
{
	// De-allocate any memory reserved in Initialize, stop any threads, etc.
	// Good practice is to write the Halt() method such that it is safe to call it even *before*
	// Initialize, and safe to call it twice (e.g. make sure you do not delete [] pointers that
	// have already been deleted:  set them to NULL after deletion).
	delete mVRTrain;
	delete mTokenPlayer;
	delete mWrongTokenPlayer;
	delete mRightTokenPlayer;
}

void
	VowelRecogTrainingTask::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
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

	// Note that the VowelRecogTrainingTask instance itself, and its members, are read-only during
	// this phase, due to the "const" at the end of the Preflight prototype above.
	// Any methods called by Preflight must also be "const" in the same way.

	Output = Input; // this simply passes information through about SampleBlock dimensions, etc....

}


void
	VowelRecogTrainingTask::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
	// The user has pressed "Set Config" and all Preflight checks have passed.
	// The signal properties can no longer be modified, but the const limitation has gone, so
	// the VowelRecogTrainingTask instance itself can be modified. Allocate any memory you need, start any
	// threads, store any information you need in private member variables.
	
	mVRTrain = new (nothrow) VowelRecogTraining;
	PreflightCondition(mVRTrain != 0);

	mTokenPlayer = new (nothrow) WavePlayer ;
	PreflightCondition(mTokenPlayer != 0);

	mWrongTokenPlayer = new (nothrow) WavePlayer ;
	PreflightCondition(mWrongTokenPlayer != 0);

	mRightTokenPlayer = new (nothrow) WavePlayer ;
	PreflightCondition(mRightTokenPlayer != 0);

	//mVRTrain->showMaximized();
	mVRTrain->HideAllWidgets(false);
	mVRTrain->showMaximized();
	mVRTrain->Initialize();

	mVRTrain->update();
}

void
	VowelRecogTrainingTask::StartRun()
{
	// The user has just pressed "Start" (or "Resume")
	bciout << "Training Started" << endl;
	mVRTrain->Start();
}


void
	VowelRecogTrainingTask::Process( const GenericSignal& Input, GenericSignal& Output )
{
	// And now we're processing a single SampleBlock of data.
	// Remember not to take too much CPU time here, or you will break the real-time constraint.
	State("TaskModes") = mVRTrain->mTaskMode;
	mVRTrain->Process();
	
	if(mVRTrain->isTaskDone)
		State("isTaskDone") = (bool) 1;
	else
		State("isTaskDone") = (bool) 0;

	if(!mVRTrain->isTaskDone && (mVRTrain->mTaskMode == m_PlayToken) && !mVRTrain->isTokenFinished)
	{
		std::string mTokenFileName(mVRTrain->WAVDIR);
		mTokenFileName.append(mVRTrain->v_TokenNamesIdx[mVRTrain->iToken]);

		//bciout << mVRTrain->mTokenName << endl;

		mTokenPlayer->SetFile(mTokenFileName);
		mTokenPlayer->Play();
		mVRTrain->isTokenFinished = true;

		State("SpeakerIdx") = mVRTrain->v_SpeakerIdx[mVRTrain->iToken];
		State("SpeakerSetIdx") = mVRTrain->v_SpeakerSetIdx[mVRTrain->iToken];
		State("SpeakerSetOptionIdx") = mVRTrain->v_OptionNosIdx[mVRTrain->iToken];
	}
	else if(mVRTrain->mTaskMode == m_SleepToEndWrongFeedback)
	{
		if((!mVRTrain->isWrongTokenPlayed) && (!mRightTokenPlayer->IsPlaying()))
		{
			mWrongTokenPlayer->SetFile(mVRTrain->mWrongTokenFullFile);
			mWrongTokenPlayer->Play();
			mVRTrain->isWrongTokenPlayed = true;

			Output = Input; // This passes the signal through unmodified.    
			return;
		}
		if((!mVRTrain->isRightTokenPlayed) && (!mWrongTokenPlayer->IsPlaying()))
		{
			mRightTokenPlayer->SetFile(mVRTrain->mRightTokenFullFile);
			mRightTokenPlayer->Play();
			mVRTrain->isRightTokenPlayed = true;

			Output = Input; // This passes the signal through unmodified.    
			return;
		}
		if(mVRTrain->isWrongTokenPlayed && (!mWrongTokenPlayer->IsPlaying()))
			mVRTrain->isWrongTokenFinished = true;
		
		if(mVRTrain->isRightTokenPlayed && (!mRightTokenPlayer->IsPlaying()))
			mVRTrain->isRightTokenFinished = true;
	}
	
	State("myScore") = mVRTrain->myScore;
	State("UserResponses") = mVRTrain->v_UserResponses[mVRTrain->iToken];

	if(mVRTrain->mTaskMode == m_SleepToEndRightFeedback)
		State("isRight") = 1;
	
	if(mVRTrain->mTaskMode == m_SleepToEndWrongFeedback)
		State("isRight") = 2;

	Output = Input; // This passes the signal through unmodified. 
}

void
	VowelRecogTrainingTask::StopRun()
{
	// The Running state has been set to 0, either because the user has pressed "Suspend",
	// or because the run has reached its natural end.
	bciout << "Training Ended" << endl;
	// You know, you can delete methods if you're not using them.
	// Remove the corresponding declaration from VowelRecogTrainingTask.h too, if so.
}
