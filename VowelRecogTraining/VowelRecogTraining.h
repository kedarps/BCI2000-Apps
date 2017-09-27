#ifndef VOWELRECOGTRAINING_H
#define VOWELRECOGTRAINING_H

#include <QtGui\QMainWindow>
#include <QtGui\QPalette>
#include <QtGui\QPushButton>

#include <QtCore\Qtime>
#include <QtCore\QString>
#include <QtCore\QThread>

#include <phonon\audiooutput.h>
#include <phonon\mediaobject.h>
#include <phonon\mediasource.h>

#include "ui_VowelRecogTraining.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <string.h>

using namespace std;

class VowelRecogTraining : public QMainWindow
{
	Q_OBJECT

private:
	Ui::VowelRecogTraining ui;
	
	// 50 Iters will work fine. Changes should be made in Initialize if N_ITERS > 50
	static const int N_ITERS = 5;
	// Time in and between states (in ms)
	static const int TYM_PRESTART=3000,
					 TYM_RIGHTFEEDBACK=1000;
	
	long int tym_Elapsed,
			 tym_ToWait;

	QTime *mTime;
	QKeyEvent *mKeyPress;
	QPalette *mColorPalette;

	void UpdateSpeakerSetsToUse(int,int);

	void PrepUIForToken(void);

	void ShowGrids(int, bool); // To show/hide individual grids
	void ShowGrids(bool); // To show/hide all grids

	void ShowFeedbackWidgets(int);// If 1: show RightWidget; 0: show WrongWidget
	void ShowFeedbackWidgets(bool);// To show/hide both widgets
	bool GiveFeedback(void);
	
public:
	VowelRecogTraining(QWidget *parent = 0, Qt::WFlags flags = 0);
	~VowelRecogTraining();
	void HideAllWidgets(bool);
	void Initialize();
	void Start(void);
	void Process(void);

	// Directory and file locations
	std::string WAVDIR;
	std::string FILE_SETSINFO;
	
	std::string mTokenName;

	std::string mRightTokenFullFile;
	std::string mWrongTokenFullFile;

	typedef enum
	{
		m_WaitUserReady,
		m_PreStart,
		m_SleepToStart,
		m_PlayToken,
		m_SleepToEndToken,
		m_SleepTillUserResponse,
		m_GiveFeedback,
		m_SleepToEndRightFeedback,
		m_SleepToEndWrongFeedback,
		m_PrepNextToken
	} e_TaskMode;
	
	e_TaskMode mTaskMode;
	
	int iToken, myScore;
	
	bool isTokenFinished,
		 isUserResponded,
		 isRightTokenPlayed,
		 isWrongTokenPlayed,
		 isRightTokenFinished,
		 isWrongTokenFinished,
		 isFeedbackOver,
		 isTaskDone,
		 isReady;

	// 0 is Wrong, 1 is Right
	std::vector<int> v_isRight;

	std::vector<int> v_SpeakerNumSets;
	std::vector<int> v_SpeakerIdx;
	std::vector<int> v_SpeakerSetIdx;
	std::vector<std::vector<int>> v_SpeakerSetsToUse;
	
	std::vector<std::vector<std::string>> v_TokenSetsIdx;
	std::vector<std::string> v_TokenNamesIdx;
	std::vector<std::string> v_TokenWordNamesIdx;
	std::vector<int> v_OptionNosIdx;

	std::vector<int> v_UserResponses;

private slots:
	// Grid 1 
	void G1_O1_Clicked();   
	void G1_O2_Clicked();   
	// Grid 2 
	void G2_O1_Clicked();   
	void G2_O2_Clicked();   
	void G2_O3_Clicked();   
	// Grid 3 
	void G3_O1_Clicked();
	void G3_O2_Clicked();   
	void G3_O3_Clicked();   
	void G3_O4_Clicked();   
	// Wrong Feedback Grid
	void ok_Clicked();
	void replay_Clicked();

	void readyClicked();
};

#endif // VOWELRECOGTRAINING_H