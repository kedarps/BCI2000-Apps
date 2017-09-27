////////////////////////////////////////////////////////////////////////////////
// $Id: $
// Authors: 
// Description: SentenceKeywordRecogTask implementation
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

#include "SentenceKeywordRecogTask.h"
#include "BCIError.h"

using namespace std;

RegisterFilter( SentenceKeywordRecogTask, 3 );
// Change the location if appropriate, to determine where your filter gets
// sorted into the chain. By convention:
//  - filter locations within SignalSource modules begin with "1."
//  - filter locations within SignalProcessing modules begin with "2."  
//       (NB: SignalProcessing modules must specify this with a Filter() command in their PipeDefinition.cpp file too)
//  - filter locations within Application modules begin with "3."


SentenceKeywordRecogTask::SentenceKeywordRecogTask()
{
	BEGIN_PARAMETER_DEFINITIONS

		"Application:SentenceKeywordRecogTask int TaskCondition= 0 0 0 4 // Select Task Condition: 0 %20 1 First%20Time 2 Unshifted%20Vocoder 3 Shifted%20Vocoder 4 Unshifted%20Vocoder%20With%20SSN (enumeration)",
		"Application:SentenceKeywordRecogTask int isTraining= 0 0 0 1 // Is Training Mode or Testing Mode? (boolean)",

		END_PARAMETER_DEFINITIONS
		// ...and likewise any state variables:

		BEGIN_STATE_DEFINITIONS

		"isTaskDone			 1 0 0 0 \r\n",
		"TaskModes			 8 0 1 0 \r\n",
		"Score			     8 0 2 0 \r\n",

		END_STATE_DEFINITIONS
}


SentenceKeywordRecogTask::~SentenceKeywordRecogTask()
{
	Halt();
}

void
	SentenceKeywordRecogTask::Halt()
{
	// De-allocate any memory reserved in Initialize, stop any threads, etc.
	// Good practice is to write the Halt() method such that it is safe to call it even *before*
	// Initialize, and safe to call it twice (e.g. make sure you do not delete [] pointers that
	// have already been deleted:  set them to NULL after deletion).
	delete mSKR;
	delete mSentencePlayer;
}

void
	SentenceKeywordRecogTask::Preflight( const SignalProperties& Input, SignalProperties& Output ) const
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

	// Note that the SentenceKeywordRecogTask instance itself, and its members, are read-only during
	// this phase, due to the "const" at the end of the Preflight prototype above.
	// Any methods called by Preflight must also be "const" in the same way.
	PreflightCondition(Parameter("TaskCondition") > 0);
	PreflightCondition(!Parameter("SubjectName").IsNull());
	PreflightCondition(!Parameter("SubjectSession").IsNull());

	Output = Input; // this simply passes information through about SampleBlock dimensions, etc....
}


void
	SentenceKeywordRecogTask::Initialize( const SignalProperties& Input, const SignalProperties& Output )
{
	// The user has pressed "Set Config" and all Preflight checks have passed.
	// The signal properties can no longer be modified, but the const limitation has gone, so
	// the SentenceKeywordRecogTask instance itself can be modified. Allocate any memory you need, start any
	// threads, store any information you need in private member variables.

	mSKR = new (nothrow) SentenceKeywordRecog;
	PreflightCondition(mSKR != 0);

	mSentencePlayer = new (nothrow) WavePlayer;
	PreflightCondition(mSentencePlayer != 0);

	mSKR->showMaximized();

	if(Parameter("isTraining") == 1)
		mSKR->isTraining = true;
	else
		mSKR->isTraining = false;

	int taskCond = Parameter("TaskCondition");
	mSKR->SUBJ_ID = Parameter("SubjectName");
	mSKR->SESSION_NO = Parameter("SubjectSession");

	mSKR->Initialize(taskCond);
	mSKR->update();
}

void
	SentenceKeywordRecogTask::StartRun()
{
	// The user has just pressed "Start" (or "Resume")
	bciout << "Hello World!" << endl;
	mSKR->Start();
}


void
	SentenceKeywordRecogTask::Process( const GenericSignal& Input, GenericSignal& Output )
{
	// And now we're processing a single SampleBlock of data.
	// Remember not to take too much CPU time here, or you will break the real-time constraint.
	mSKR->Process();

	State("TaskModes") = mSKR->mTaskMode;
	State("Score") = mSKR->myScore;

	if(mSKR->isTaskDone)
		State("isTaskDone") = (bool) 1;
	else
		State("isTaskDone") = (bool) 0;

	if(!mSKR->isTaskDone && ((mSKR->mTaskMode == m_PlaySentence) || (mSKR->mTaskMode == m_RepeatSentence) || (mSKR->mTaskMode == m_SleepTillFeedbackOver)) && !mSKR->isSentencePlayed)
	{
		std::string mSentenceFileName(mSKR->WAV_DIR);
		mSentenceFileName.append(mSKR->v_SntcWavFileNames[mSKR->iSentence]);

		mSentencePlayer->SetFile(mSentenceFileName);
		mSentencePlayer->Play();
		mSKR->isSentencePlayed = true;

		if(mSKR->mTaskMode == m_PlaySentence)
			bciout << mSKR->v_SntcWavFileNames[mSKR->iSentence] << endl;
	}
	if(mSKR->isSentencePlayed && !mSentencePlayer->IsPlaying())
	{
		mSKR->isSentenceFinished = true;
	}

	if(mSKR->isTaskDone && !mSKR->isSavedResults)
	{
		/*mSKR->SaveResults();*/
		mSKR->isSavedResults = true;

		bciout << mSKR->SESSION_DIR << endl;
		bciout << mSKR->RUN_NO << endl;
		bciout << mSKR->Run_No << endl;
		bciout << mSKR->TXT_FILE << endl;
	}
	Output = Input; // This passes the signal through unmodified.                  
}

void
	SentenceKeywordRecogTask::StopRun()
{
	// The Running state has been set to 0, either because the user has pressed "Suspend",
	// or because the run has reached its natural end.
	bciout << "Goodbye World." << endl;
	// You know, you can delete methods if you're not using them.
	// Remove the corresponding declaration from SentenceKeywordRecogTask.h too, if so.
}