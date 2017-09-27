#ifndef SENTENCEKEYWORDRECOG_H
#define SENTENCEKEYWORDRECOG_H

#include <QtGui\QMainWindow>
#include <QtGui\QPalette>
#include <QtGui\QPushButton>
#include <QLayoutItem>
#include <QLineEdit>

#include <QtCore\Qtime>
#include <QtCore\QString>
#include <QtCore\QThread>

#include <QKeyEvent>

#include "ui_SentenceKeywordRecog.h"

#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <vector>
#include <dirent.h>
#include <string>
#include <unordered_map>

using namespace std;

class SentenceKeywordRecog : public QMainWindow
{
	Q_OBJECT

private:
	Ui::SentenceKeywordRecog ui;

	int N_ITERS;
	// Time in and between states (in ms)
	static const int TYM_PRESTART=3000,
					 TYM_SLEEPPRENEXTSENTENCE=1000;
	
	long int tym_Elapsed,
		tym_ToWait;

	QTime *mTime;
	std::string database;

	QVector<QLineEdit*> qv_CurrentLineEdits;
	QVector<QLabel*> qv_CurrentDidYouMeans;
	QVector<QLabel*> qv_CurrentCorrectLabels;
	QVector<QPushButton*> qv_CurrentYesButtons;
	QVector<QLabel*> qv_CurrentFeedbackLabels;
	QVector<QPushButton*> qv_CurrentRightOrWrong;

	QWidget* userEditWidget;
	
	int widgetWidth;
	int widgetHeight;

	// Spell Corrector Related
	typedef std::vector<std::string> sp_Vector;
    typedef std::unordered_map<std::string, int> sp_Dictionary;
 
    sp_Dictionary sp_dict;
 
    void sp_Edits(const std::string& word, sp_Vector& result);
    void sp_Known(sp_Vector& results, sp_Dictionary& candidates);
    void sp_Load(const std::string& filename);
    std::string sp_Correct(const std::string& word);

public:
	SentenceKeywordRecog(QWidget *parent = 0, Qt::WFlags flags = 0);
	~SentenceKeywordRecog();
	void UpdateScore(void);
	void Initialize(int);
	void GetSessionAndRunInfo(void);
	void PrepUIForUser(void);
	void RunSpellCheck(void);
	void GiveFeedback(void);
	void PrepNextSentence(void);
	void CheckForAllUserResponses(void);
	void Start(void);
	void Process(void);
	void SaveResults(void);

	// Directory where sound files live
	std::string BCI2000_ROOT_DIR;
	std::string SNTC_ROOT_DIR;
	std::string WAV_DIR;
	std::string TXT_FILE;

	// Session and Run Numbers
	std::string SUBJ_ID;
	std::string SESSION_NO;
	std::string RUN_NO;
	int Run_No;
	std::string SESSION_DIR;
	std::string DATA_DIR;

	typedef enum
	{
		m_WaitUserReady,
		m_PreStart,
		m_SleepToStart,
		m_PlaySentence,
		m_SleepToEndSentence,
		m_SleepTillUserResponse,
		m_RunSpellCheck,
		m_SleepTillSpellCheckDone,
		m_PrepFeedback,
		m_SleepTillFeedbackOver,
		m_PrepSleepPreNextSentence,
		m_SleepPreNextSentence,
		m_PrepNextSentence,

		m_RepeatSentence,
		m_SleepToEndRepeatedSentence
	} e_TaskMode;
	
	e_TaskMode mTaskMode;
	
	int iSentence, nCorrectResponses, nMaxResponses, myScore ,nRepeats, nMaxRepeats;
	
	bool isTraining,
		 isSentencePlayed,
		 isSentenceFinished,
		 isUserResponded,
		 isSpellCheckDone,
		 isTaskDone,
		 isReady,
		 isSavedResults;
	
	std::vector<dirent> v_AllSentenceWavs;
	std::vector<int> v_SntcWavsIdx;
	std::vector<std::string> v_AllNonKeywords;
	std::vector<std::string> v_SntcWavFileNames;
	std::vector<std::vector<std::string>> vv_SntcWavsTokens;
	
	std::vector<std::vector<std::string>> vv_UserResponses;
	std::vector<bool> v_isRight;

private slots:
	void onClickReady();
	void onClickRepeat();
	void onClickDone();
	void onClickYes();

protected:
	void keyReleaseEvent(QKeyEvent*);
};

#endif // SENTENCEKEYWORDRECOG_H