#include "SentenceKeywordRecog.h"

SentenceKeywordRecog::SentenceKeywordRecog(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags),
	BCI2000_ROOT_DIR("C:\\BCI2000\\v4369\\"),
	
	iSentence(0),
	
	nCorrectResponses(0),
	nMaxResponses(0),
	myScore(0),

	nRepeats(0),
	nMaxRepeats(3),

	isSentencePlayed(false),
	isSentenceFinished(false),
	isUserResponded(false),
	isSpellCheckDone(false),
	isTaskDone(false),
	isReady(false),
	isSavedResults(false),

	mTime(new QTime()),
	widgetWidth(1200),
	widgetHeight(200)
{
	ui.setupUi(this);
	// Connect button click SLOTS
	connect(ui.ready,SIGNAL(clicked()),this,SLOT(onClickReady()));
	connect(ui.done,SIGNAL(clicked()),this,SLOT(onClickDone()));
	connect(ui.repeat,SIGNAL(clicked()),this,SLOT(onClickRepeat()));

	userEditWidget = new QWidget(ui.MainWidget);
	
	DATA_DIR = BCI2000_ROOT_DIR;
	DATA_DIR.append("data\\TrainingTask_NH2\\VRTest\\");

	SNTC_ROOT_DIR = BCI2000_ROOT_DIR;
	SNTC_ROOT_DIR.append("prog\\sounds\\SentenceKeywordRecog\\Sentences_");
}

SentenceKeywordRecog::~SentenceKeywordRecog()
{
	delete mTime;
}

void SentenceKeywordRecog::Initialize(int taskCond)
{
	ui.instr_1->setVisible(false);
	ui.instr_2->setVisible(false);
	ui.instr_3->setVisible(false);

	ui.time->setVisible(false);
	ui.play->setVisible(false);
	ui.done->setVisible(false);

	ui.ready->setVisible(true);
	ui.repeat->setVisible(false);

	ui.status->move(ui.MainWidget->geometry().center().x() - ui.status->geometry().width()/2, 0);
	QString playLoc = QString::fromStdString(BCI2000_ROOT_DIR);
	ui.play->setPixmap(*(new QPixmap(playLoc.append("src\\custom\\SentenceKeywordRecog\\play.png"))));
	ui.play->move(ui.MainWidget->geometry().center().x() - ui.play->geometry().width()/2, ui.status->geometry().height());
	ui.repeat->move(ui.MainWidget->geometry().center().x() - ui.repeat->geometry().width()/2, ui.status->geometry().height());

	QString repTxt("Repeat\n[");
	repTxt.append(QString::number(nMaxRepeats));
	repTxt.append(" left]");
	ui.repeat->setText(repTxt);
	
	ui.ready->move(ui.MainWidget->geometry().center().x() - ui.ready->geometry().width()/2, ui.status->geometry().height());
	ui.done->move(ui.MainWidget->geometry().center().x() - ui.done->geometry().width()/2, 700);
	ui.score->move(ui.MainWidget->geometry().center().x() - ui.score->geometry().width()/2, 800);

	if(isTraining)
	{
		database = "IEEE";
		N_ITERS = 5;
	}
	else
	{
		database = "CUNY";
		N_ITERS = 5;
		QString statTxt = "Testing Run.\nPress button when Ready to start";
		ui.status->setText(statTxt);
	}

	SNTC_ROOT_DIR.append(database);
	SNTC_ROOT_DIR.append("\\");

	WAV_DIR = SNTC_ROOT_DIR;
	
	switch(taskCond)
	{
	case 1://First Time
		WAV_DIR.append("sentences\\");
		break;
	case 2://Unshifted
		WAV_DIR.append("sentences_vocoded_unshifted\\");
		break;
	case 3://Shifted
		WAV_DIR.append("sentences_vocoded_shifted\\");
		break;
	case 4://Unshifted with SSN
		WAV_DIR.append("sentences_vocoded_ssn\\");
		break;
	}
	
	std::string WORDS_FILE = SNTC_ROOT_DIR;
	WORDS_FILE.append(database);
	WORDS_FILE.append("_UniqueWords.txt");

	sp_Load(WORDS_FILE);

	std::string SNTC_INFO_FILE = SNTC_ROOT_DIR,
		NON_KEY_FILE = SNTC_ROOT_DIR;

	/*****************Populate Sound Files********************/
	DIR *mDir;
	struct dirent *tSoundFile;

	mDir = opendir(WAV_DIR.c_str());

	if(mDir == NULL)
		return;

	while((tSoundFile = readdir(mDir)) != NULL)
	{
		// Make sure we get only sound files
		if(strstr(tSoundFile->d_name,".wav"))
			v_AllSentenceWavs.push_back(*tSoundFile);
		else
			continue;
	}

	closedir(mDir);

	/*****************Make Non-Keywords Vector********************/
	NON_KEY_FILE.append(database);
	NON_KEY_FILE.append("_NonKeywords.txt");
	std::ifstream nonKeyInfoFile;
	nonKeyInfoFile.open(NON_KEY_FILE);

	std::string currLine;

	while(std::getline(nonKeyInfoFile,currLine))
	{
		std::string thisToken;
		v_AllNonKeywords.push_back(currLine.substr(0,currLine.find_first_of(" ")));
	}

	/***********************Main Task Init********************/
	SNTC_INFO_FILE.append(database);
	SNTC_INFO_FILE.append("_SentenceInfo.txt");

	srand((unsigned) time(NULL));
	int nSentences = v_AllSentenceWavs.size();

	// Lets get unique sentences for each iteration
	for(int iIter = 0; iIter < N_ITERS; iIter++)
	{
		int idxSntcHere;

		if(iIter == 0)
			idxSntcHere = rand() % nSentences;
		else
		{
			do idxSntcHere = rand() % nSentences;
			while(std::find(v_SntcWavsIdx.begin(),v_SntcWavsIdx.end(),idxSntcHere) != v_SntcWavsIdx.end());
		}
		v_SntcWavsIdx.push_back(idxSntcHere);

		std::string wavFileName = v_AllSentenceWavs[v_SntcWavsIdx[iIter]].d_name;
		v_SntcWavFileNames.push_back(wavFileName);

		// Do text search to find sentence info
		std::string txtToSearch = wavFileName.substr(0,wavFileName.find_first_of(".wav"));
		std::ifstream sntcInfoFile;
		sntcInfoFile.open(SNTC_INFO_FILE);

		std::string currLine;
		std::vector<std::string> v_ThisSntcTokens;

		while(std::getline(sntcInfoFile, currLine))
		{
			if (currLine.find(txtToSearch, 0) != std::string::npos)
			{
				std::string thisToken;

				// Now that you have the current line, extract the token names in given sentence
				std::string AllTokensThisSntc = currLine.substr(currLine.find_first_of(" - ")+3);
				std::istringstream iss(AllTokensThisSntc);

				while(std::getline(iss,thisToken,' '))
					v_ThisSntcTokens.push_back(thisToken);

				break;
			}
		}
		vv_SntcWavsTokens.push_back(v_ThisSntcTokens);
	}

	mTaskMode = m_WaitUserReady;
}

void SentenceKeywordRecog::GetSessionAndRunInfo(void)
{
	/*SESSION_DIR = BCI2000_ROOT_DIR;
	SESSION_DIR.append("data\\TrainingTask_NH2\\VRTest\\");*/
	SESSION_DIR = DATA_DIR;
	SESSION_DIR.append(SUBJ_ID);
	SESSION_DIR.append(SESSION_NO);
	SESSION_DIR.append("\\");

	/*****************Populate Dat Files********************/
	DIR *mDir;
	struct dirent *tDatFile;
	std::vector<dirent> v_AllDatFiles;

	mDir = opendir(SESSION_DIR.c_str());

	if(mDir == NULL)
		return;

	while((tDatFile = readdir(mDir)) != NULL)
	{
		// Make sure we get only sound files
		if(strstr(tDatFile->d_name,".dat"))
			v_AllDatFiles.push_back(*tDatFile);
		else
			continue;
	}
	closedir(mDir);

	int maxRunNo = 0;
	for(unsigned int iFile = 0; iFile < v_AllDatFiles.size();iFile++)
	{
		std::string fNameHere = v_AllDatFiles[iFile].d_name;
		fNameHere = fNameHere.substr(0,fNameHere.find_last_of("."));
		std::string runNoHereStr = fNameHere.substr(fNameHere.find_last_of("R")+1);

		int runNoHereInt = std::stoi(runNoHereStr);

		if(runNoHereInt >= maxRunNo)
			maxRunNo = runNoHereInt;
	}
	Run_No = maxRunNo++;
	RUN_NO = std::to_string(static_cast<long long> (Run_No));
}

void SentenceKeywordRecog::Start(void)
{
	mTime->start();
}

void SentenceKeywordRecog::Process(void)
{
	QString startTxt, statusTxt, repTxt;

	repTxt.append("Repeat\n[");
	tym_Elapsed = mTime->elapsed();
	ui.time->display(tym_Elapsed);

	switch(mTaskMode)
	{
	case m_WaitUserReady://0
		mTaskMode = isReady ? m_PreStart : m_WaitUserReady;
		break;
	case m_PreStart://1
		ui.ready->setVisible(false);
		tym_ToWait = tym_Elapsed + TYM_PRESTART;
		mTaskMode = m_SleepToStart;
		break;
	case m_SleepToStart://2
		if(((tym_ToWait - tym_Elapsed) > 2000) & ((tym_ToWait - tym_Elapsed) < 3000))
			startTxt.append("Starting in 3");
		else if(((tym_ToWait - tym_Elapsed) > 1000) & ((tym_ToWait - tym_Elapsed) < 2000))
			startTxt.append("Starting in 2");
		else if((tym_ToWait - tym_Elapsed) < 1000)
			startTxt.append("Starting in 1");

		ui.status->setText(startTxt);

		mTaskMode = ((tym_ToWait - tym_Elapsed) < 10) ? m_PlaySentence : m_SleepToStart;
		break;
	case m_PlaySentence:
		if(isTaskDone)
		{
			statusTxt = "End of";
			
			if(isTraining)
				statusTxt.append(" Training Run");
			else
				statusTxt.append(" Testing Run");
				
			ui.status->setText(statusTxt);
			break;
		}
		else
		{
			statusTxt.append("Trial ");
			statusTxt.append(QString::number(iSentence+1));
			statusTxt.append(" of ");
			statusTxt.append(QString::number(N_ITERS));

			ui.play->setVisible(false);

			ui.status->setText(statusTxt);
			mTaskMode = m_SleepToEndSentence;
			break;
		}
	case m_SleepToEndSentence:
		if(isSentenceFinished)
		{
			ui.repeat->setVisible(true);
			ui.play->setVisible(false);
			PrepUIForUser();
			mTaskMode = m_SleepTillUserResponse;
		}
		else
		{
			ui.play->setVisible(true);
			mTaskMode = m_SleepToEndSentence;
		}
		break;
	case m_SleepTillUserResponse:
		CheckForAllUserResponses();
		mTaskMode = isUserResponded ? m_RunSpellCheck : m_SleepTillUserResponse;
		break;
	case m_RunSpellCheck:
		ui.repeat->setVisible(false);
		RunSpellCheck();
		ui.done->setText("Done");
		mTaskMode = m_SleepTillSpellCheckDone;
		break;
	case m_SleepTillSpellCheckDone:
		if(isSpellCheckDone)
		{
			ui.done->setVisible(false);
			if(isTraining)
				mTaskMode = m_PrepFeedback;
			else
				mTaskMode = m_PrepSleepPreNextSentence;
		}
		else
			mTaskMode = m_SleepTillSpellCheckDone;
		break;
	case m_PrepFeedback:
		GiveFeedback();
		isSentencePlayed = false;
		isSentenceFinished = false;
		ui.play->setVisible(true);
		mTaskMode = m_SleepTillFeedbackOver;
		break;
	case m_SleepTillFeedbackOver:
		if(isSentenceFinished)
		{
			ui.play->setVisible(false);
			mTaskMode = m_PrepSleepPreNextSentence;
		}
		else
			mTaskMode = m_SleepTillFeedbackOver;
		break;
	case m_PrepSleepPreNextSentence:
		UpdateScore();
		tym_ToWait = tym_Elapsed + TYM_SLEEPPRENEXTSENTENCE;
		mTaskMode = m_SleepPreNextSentence;
		break;
	case m_SleepPreNextSentence:
		mTaskMode = ((tym_ToWait - tym_Elapsed) < 10) ? m_PrepNextSentence : m_SleepPreNextSentence;
		break;
	case m_PrepNextSentence:
		PrepNextSentence();
		iSentence++;
		isSentenceFinished = false;
		isSentencePlayed = false;
		isUserResponded = false;
		isSpellCheckDone = false;
		nRepeats = 0;
		repTxt.append(QString::number(nMaxRepeats));
		repTxt.append(" left]");
		ui.repeat->setText(repTxt);

		if(iSentence == N_ITERS)
			isTaskDone = true;

		mTaskMode = m_PlaySentence;
		break;
	case m_RepeatSentence:
		ui.repeat->setVisible(false);
		ui.play->setVisible(true);
		mTaskMode = isSentencePlayed ? m_SleepToEndRepeatedSentence: m_RepeatSentence;
		break;
	case m_SleepToEndRepeatedSentence:
		if(isSentenceFinished)
		{
			if(nRepeats < nMaxRepeats)
				ui.repeat->setVisible(true);
			else
				ui.repeat->setVisible(false);

			ui.play->setVisible(false);
			mTaskMode = m_SleepTillUserResponse;
		}
		else
		{
			ui.play->setVisible(true);
			mTaskMode = m_SleepToEndRepeatedSentence;
		}
		break;
	}
}

void SentenceKeywordRecog::PrepNextSentence(void)
{
	QLayoutItem *child;
	while ((child = ui.userEditLayout->takeAt(0)) != NULL) 
	{
		delete child->widget();
		delete child;
	}

	userEditWidget->hide();

	if(!isTraining)
	{
		for(int iDidYouMean = 0; iDidYouMean < qv_CurrentDidYouMeans.size(); iDidYouMean++)
		{
			qv_CurrentDidYouMeans[iDidYouMean]->setVisible(false);
			qv_CurrentCorrectLabels[iDidYouMean]->setVisible(false);
			qv_CurrentYesButtons[iDidYouMean]->setVisible(false);
		}
	}
	
	qv_CurrentLineEdits.erase(qv_CurrentLineEdits.begin(),qv_CurrentLineEdits.end());
	qv_CurrentDidYouMeans.erase(qv_CurrentDidYouMeans.begin(),qv_CurrentDidYouMeans.end());
	qv_CurrentCorrectLabels.erase(qv_CurrentCorrectLabels.begin(),qv_CurrentCorrectLabels.end());
	qv_CurrentYesButtons.erase(qv_CurrentYesButtons.begin(),qv_CurrentYesButtons.end());
	
	if(isTraining)
	{
		for(int iLabel = 0; iLabel < qv_CurrentFeedbackLabels.size(); iLabel++)
		{
			qv_CurrentFeedbackLabels[iLabel]->setVisible(false);
			qv_CurrentRightOrWrong[iLabel]->setVisible(false);
		}
		qv_CurrentFeedbackLabels.erase(qv_CurrentFeedbackLabels.begin(),qv_CurrentFeedbackLabels.end());
		qv_CurrentRightOrWrong.erase(qv_CurrentRightOrWrong.begin(),qv_CurrentRightOrWrong.end());
	}
	ui.done->setVisible(false);
}

void SentenceKeywordRecog::PrepUIForUser(void)
{
	std::vector<std::string> v_ThisSntcTokens = vv_SntcWavsTokens[iSentence];

	for(unsigned int iToken = 0; iToken < v_ThisSntcTokens.size(); iToken++)
	{
		std::string thisToken = v_ThisSntcTokens[iToken];

		// Convert to lower case
		std::string thisTokenLower(thisToken);
		std::transform(thisTokenLower.begin(), thisTokenLower.end(), thisTokenLower.begin(), ::tolower);
		// We find if given token is a non-keyword
		bool isTokenNonKeyword = std::find(v_AllNonKeywords.begin(),v_AllNonKeywords.end(), thisTokenLower) != v_AllNonKeywords.end();

		if(isTokenNonKeyword)
		{
			QLabel *mLabel = new QLabel(QString::fromStdString(thisToken));
			QFont mFont = mLabel->font();
			mFont.setPointSize(20);
			mLabel->setFont(mFont);
			ui.userEditLayout->addWidget(mLabel);
		}
		else
		{
			QLineEdit *mLineEdit = new QLineEdit();
			mLineEdit->setMaxLength(20);
			QFont mFont = mLineEdit->font();
			mFont.setPointSize(20);
			mLineEdit->setFont(mFont);
			mLineEdit->setAlignment(Qt::AlignCenter);
			ui.userEditLayout->addWidget(mLineEdit);

			qv_CurrentLineEdits.push_back(mLineEdit);
		}
		ui.userEditLayout->addSpacing(10);
	}
	qv_CurrentLineEdits[0]->setFocus();
	userEditWidget->setLayout(ui.userEditLayout);

	userEditWidget->setGeometry(ui.MainWidget->geometry().center().x()-widgetWidth/2, ui.MainWidget->geometry().center().y()-widgetHeight/2, widgetWidth, widgetHeight);
	userEditWidget->show();
}

void SentenceKeywordRecog::RunSpellCheck(void)
{
	QPoint userEditWidgetTopLeft = userEditWidget->geometry().topLeft();

	for(int iLineEdit = 0; iLineEdit < qv_CurrentLineEdits.size(); iLineEdit++)
	{
		QWidget *spellCheckWidget = new QWidget(ui.MainWidget);

		QLineEdit *thisLineEdit = qv_CurrentLineEdits[iLineEdit];
		QString userWord = thisLineEdit->text();
		thisLineEdit->setDisabled(true);
		std::string spellCheckedWord(sp_Correct(userWord.toStdString()));

		QPoint lineEditTopLeft = thisLineEdit->geometry().topLeft();
		QLabel *didYouMean = new QLabel("Did you mean?");
		QLabel *correctedWord = new QLabel(QString::fromStdString(spellCheckedWord));

		didYouMean->setAlignment(Qt::AlignCenter);
		correctedWord->setAlignment(Qt::AlignCenter);
		QPushButton *yes = new QPushButton("Yes");

		QFont mFont = yes->font();
		mFont.setPointSize(12);
		didYouMean->setFont(mFont);
		yes->setFont(mFont);
		QPalette *mPalette = new QPalette;
		mPalette->setColor(QPalette::WindowText,Qt::red);
		correctedWord->setPalette(*mPalette);

		correctedWord->setFont(mFont);

		qv_CurrentDidYouMeans.push_back(didYouMean);
		qv_CurrentCorrectLabels.push_back(correctedWord);
		qv_CurrentYesButtons.push_back(yes);

		QGridLayout *layout = new QGridLayout;
		layout->addWidget(didYouMean, 0, 0, 1, 2);
		layout->addWidget(correctedWord, 1, 0, 1, 2);
		layout->addWidget(yes, 2, 0, 1, 2);

		if(!spellCheckedWord.compare(userWord.toStdString()) || spellCheckedWord.empty())
		{
			didYouMean->setText("No suggestions");
			correctedWord->setVisible(false);
			yes->setVisible(false);
		}

		QRect widgetGeometry(userEditWidgetTopLeft.x()+lineEditTopLeft.x(), 
			userEditWidgetTopLeft.y()+lineEditTopLeft.y()+thisLineEdit->geometry().height(), 
			thisLineEdit->geometry().width(),
			thisLineEdit->geometry().height()+50);

		spellCheckWidget->setGeometry(widgetGeometry);
		spellCheckWidget->setLayout(layout);
		spellCheckWidget->show();
	}
	// For current yes buttons connect slots 
	for(int iButton = 0; iButton < qv_CurrentYesButtons.size(); iButton++)
		connect(qv_CurrentYesButtons[iButton],SIGNAL(clicked()),this,SLOT(onClickYes()));
}

void SentenceKeywordRecog::CheckForAllUserResponses(void)
{
	std::vector<bool> v_allEditsDone;
	bool allEditsDone = true;	

	for(int iLineEdit = 0; iLineEdit < qv_CurrentLineEdits.size(); iLineEdit++)
	{
		if(qv_CurrentLineEdits[iLineEdit]->text().isEmpty())
			v_allEditsDone.push_back(false);
		else
			v_allEditsDone.push_back(true);
	}

	for(unsigned int iEdit = 0; iEdit < v_allEditsDone.size(); iEdit++)
		allEditsDone &= v_allEditsDone[iEdit];
	
	ui.done->setText("Spell Check");
	ui.done->setVisible(allEditsDone);
}

void SentenceKeywordRecog::GiveFeedback(void)
{
	for(int iDidYouMean = 0; iDidYouMean < qv_CurrentDidYouMeans.size(); iDidYouMean++)
	{
		qv_CurrentDidYouMeans[iDidYouMean]->setVisible(false);
		qv_CurrentCorrectLabels[iDidYouMean]->setVisible(false);
		qv_CurrentYesButtons[iDidYouMean]->setVisible(false);
	}

	QPoint userEditWidgetTopLeft = userEditWidget->geometry().topLeft();
	std::vector<std::string> v_ThisSntcTokens = vv_SntcWavsTokens[iSentence];
	int iLineEdit = 0;

	for(unsigned int iToken = 0; iToken < v_ThisSntcTokens.size(); iToken++)
	{
		std::string thisToken = v_ThisSntcTokens[iToken];

		// Convert to lower case
		std::string thisTokenLower(thisToken);
		std::transform(thisTokenLower.begin(), thisTokenLower.end(), thisTokenLower.begin(), ::tolower);
		// We find if given token is a non-keyword
		bool isTokenNonKeyword = std::find(v_AllNonKeywords.begin(),v_AllNonKeywords.end(), thisTokenLower) != v_AllNonKeywords.end();

		if(isTokenNonKeyword)
			continue;
		else
		{
			QWidget *feedbackWidget = new QWidget(ui.MainWidget);
			QLineEdit *thisLineEdit = qv_CurrentLineEdits[iLineEdit];
			QPoint thisLineEditTopLeft = thisLineEdit->geometry().topLeft();
			std::string userResponseLower = thisLineEdit->text().toStdString();
			std::transform(userResponseLower.begin(), userResponseLower.end(), userResponseLower.begin(), ::tolower);

			QLabel *mLabel = new QLabel(QString::fromStdString(thisToken));
			QPushButton *mRightOrWrong = new QPushButton();
			mRightOrWrong->setEnabled(false);
			
			mLabel->setAlignment(Qt::AlignCenter);
			QFont mFont = mLabel->font();
			mFont.setPointSize(20);
			mLabel->setFont(mFont);

			mFont.setPointSize(14);
			mRightOrWrong->setFont(mFont);

			QPalette *mPalette = new QPalette;
			QString correctLoc = QString::fromStdString(BCI2000_ROOT_DIR);
			QString incorrectLoc = QString::fromStdString(BCI2000_ROOT_DIR);

			if(!thisTokenLower.compare(userResponseLower))
			{
				mRightOrWrong->setText("Right");
				mRightOrWrong->setIcon(QIcon(QPixmap(correctLoc.append("src\\custom\\SentenceKeywordRecog\\correct.png"))));	
				mPalette->setColor(QPalette::WindowText,Qt::green);
			}
			else
			{
				mRightOrWrong->setText("Wrong");
				mRightOrWrong->setIcon(QIcon(QPixmap(incorrectLoc.append("src\\custom\\SentenceKeywordRecog\\incorrect.png"))));	
				mPalette->setColor(QPalette::WindowText,Qt::red);
			}
			mLabel->setPalette(*mPalette);
			
			QGridLayout *layout = new QGridLayout;
			layout->addWidget(mLabel, 0, 0, 1, 2);
			layout->addWidget(mRightOrWrong, 1, 0, 1, 2);
				
			QRect widgetGeometry(userEditWidgetTopLeft.x()+thisLineEditTopLeft.x(), 
				userEditWidgetTopLeft.y()+thisLineEditTopLeft.y()+thisLineEdit->geometry().height(), 
				thisLineEdit->geometry().width(),
				thisLineEdit->geometry().height()+50);

			feedbackWidget->setGeometry(widgetGeometry);
			feedbackWidget->setLayout(layout);
			feedbackWidget->show();

			qv_CurrentFeedbackLabels.push_back(mLabel);
			qv_CurrentRightOrWrong.push_back(mRightOrWrong);

			iLineEdit++;
		}

	}

}

void SentenceKeywordRecog::UpdateScore(void)
{
	std::vector<std::string> v_ThisSntcTokens = vv_SntcWavsTokens[iSentence];
	std::vector<std::string> v_ThisSntcUserResp;

	int iLineEdit = 0;

	for(unsigned int iToken = 0; iToken < v_ThisSntcTokens.size(); iToken++)
	{
		std::string thisToken = v_ThisSntcTokens[iToken];

		// Convert to lower case
		std::string thisTokenLower(thisToken);
		std::transform(thisTokenLower.begin(), thisTokenLower.end(), thisTokenLower.begin(), ::tolower);
		// We find if given token is a non-keyword
		bool isTokenNonKeyword = std::find(v_AllNonKeywords.begin(),v_AllNonKeywords.end(), thisTokenLower) != v_AllNonKeywords.end();

		if(isTokenNonKeyword)
			continue;
		else
		{
			QLineEdit *thisLineEdit = qv_CurrentLineEdits[iLineEdit];
			std::string userResponseLower = thisLineEdit->text().toStdString();
			std::transform(userResponseLower.begin(), userResponseLower.end(), userResponseLower.begin(), ::tolower);
			v_ThisSntcUserResp.push_back(userResponseLower);

			if(!thisTokenLower.compare(userResponseLower))
				nCorrectResponses++;

			nMaxResponses++;
			iLineEdit++;
		}
	}
	vv_UserResponses.push_back(v_ThisSntcUserResp);

	QString scoreTxt = "Your Score - ";
	myScore = 100*((double) nCorrectResponses/(double) nMaxResponses);
	scoreTxt.append(QString::number(myScore));
	scoreTxt.append(" %");
	ui.score->setText(scoreTxt);
}

void SentenceKeywordRecog::onClickReady()
{
	isReady = true;
}

void SentenceKeywordRecog::onClickYes()
{
	QObject* senderObj = QObject::sender();

	for(int iYes = 0; iYes < qv_CurrentYesButtons.size(); iYes++)
	{
		if(senderObj == qv_CurrentYesButtons[iYes])
		{
			qv_CurrentLineEdits[iYes]->setText(qv_CurrentCorrectLabels[iYes]->text());
		}
	}
}

void SentenceKeywordRecog::onClickDone()
{
	if(!isUserResponded)
		isUserResponded = true;
	else if(!isSpellCheckDone)
		isSpellCheckDone = true;
}

void SentenceKeywordRecog::onClickRepeat()
{
	nRepeats++;
	isSentencePlayed = false;
	isSentenceFinished = false;

	QString repText = "Repeat\n[";
	repText.append(QString::number(nMaxRepeats - nRepeats));
	repText.append(" left]");
	ui.repeat->setText(repText);

	mTaskMode = m_RepeatSentence;
}

void SentenceKeywordRecog::keyReleaseEvent(QKeyEvent *mKeyEvent)
{
	if((mTaskMode == m_SleepTillUserResponse) || (mTaskMode == m_RunSpellCheck) || (mTaskMode == m_SleepTillSpellCheckDone))
	{
		if(ui.done->isVisible())
		{
			if((mKeyEvent->key() == Qt::Key_Enter) || (mKeyEvent->key() == Qt::Key_Return))
				ui.done->click();
			else
				return;
		}
		else if(ui.repeat->isVisible())
		{
			if((mKeyEvent->key() == Qt::Key_Enter) || (mKeyEvent->key() == Qt::Key_Return))
				ui.repeat->click();
			else
				return;
		}
		else
			return;
	}
	else if(ui.ready->isVisible())
	{
		if((mKeyEvent->key() == Qt::Key_Enter) || (mKeyEvent->key() == Qt::Key_Return))
			ui.ready->click();
		else
			return;
	}
	else
		return;
}

void SentenceKeywordRecog::SaveResults(void)
{
	/*SESSION_DIR = BCI2000_ROOT_DIR;
	SESSION_DIR.append("data\\");*/
	SESSION_DIR = DATA_DIR;
	SESSION_DIR.append(SUBJ_ID);
	SESSION_DIR.append(SESSION_NO);
	SESSION_DIR.append("\\");

	std::string saveToTxtFileName = SESSION_DIR;
	saveToTxtFileName.append(SUBJ_ID);
	saveToTxtFileName.append("S");
	saveToTxtFileName.append(SESSION_NO);
	saveToTxtFileName.append("R");
	saveToTxtFileName.append(RUN_NO);
	saveToTxtFileName.append(".txt");

	TXT_FILE = saveToTxtFileName;

	std::ofstream saveToTxtFile;
	saveToTxtFile.open(saveToTxtFileName);

	for(unsigned int iSntc = 0; iSntc < vv_SntcWavsTokens.size(); iSntc++)
	{
		std::vector<std::string> v_ThisSntcTokens = vv_SntcWavsTokens[iSntc];
		std::vector<std::string> v_ThisSntcUserResp = vv_UserResponses[iSntc];

		int iUserResp = 0;

		for(unsigned int iToken = 0; iToken < v_ThisSntcTokens.size(); iToken++)
		{
			std::string thisToken = v_ThisSntcTokens[iToken];
			// Convert to lower case
			std::string thisTokenLower(thisToken);
			std::transform(thisTokenLower.begin(), thisTokenLower.end(), thisTokenLower.begin(), ::tolower);
			// We find if given token is a non-keyword
			bool isTokenNonKeyword = std::find(v_AllNonKeywords.begin(),v_AllNonKeywords.end(), thisTokenLower) != v_AllNonKeywords.end();

			if(isTokenNonKeyword)
				continue;
			else
			{
				std::string thisUserResp = v_ThisSntcUserResp[iUserResp];
				std::string thisUserRespLower(thisUserResp);
				std::transform(thisUserRespLower.begin(), thisUserRespLower.end(), thisUserRespLower.begin(), ::tolower);

				saveToTxtFile << thisTokenLower;
				saveToTxtFile << "-";
				saveToTxtFile << thisUserRespLower;
				saveToTxtFile << "\n";
				iUserResp++;
			}
		}
		saveToTxtFile << "-------\n";
	}
	saveToTxtFile.close();
	saveToTxtFile.clear();
}

/****************************** SpellCorrector *****************************/
/*
 * SpellCorrector.cpp
 *
 * Copyright  (C)  2007  Felipe Farinon <felipe.farinon@gmail.com>
 *
 * Version: 1.4
 * Author: Felipe Farinon <felipe.farinon@gmail.com>
 * Maintainer: Felipe Farinon <felipe.farinon@gmail.com>
 * URL: http://scarvenger.wordpress.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Commentary:
 *
 * See http://scarvenger.wordpress.com/.
 *
 * Code:
 */
bool sortBySecond(const pair<std::string, int>& left, const pair<std::string, int>& right)
{
	return left.second < right.second;
}

void SentenceKeywordRecog::sp_Load(const std::string& filename)
{
	// Build Hash-Table
	std::ifstream wordsFile;
	wordsFile.open(filename.c_str());

	std::string currLine;

	while(std::getline(wordsFile, currLine))
	{
		std::string thisWord = currLine.substr(0,currLine.find_first_of(" "));
		// This will find unique entries in the list of words
		sp_dict[thisWord]++;
	}
	wordsFile.close();
	wordsFile.clear();
}

string SentenceKeywordRecog::sp_Correct(const std::string& word)
{
	sp_Vector result;
	sp_Dictionary candidates;

	if (sp_dict.find(word) != sp_dict.end()) { return word; }

	sp_Edits(word, result);
	sp_Known(result, candidates);

	if (candidates.size() > 0) { return max_element(candidates.begin(), candidates.end(), sortBySecond)->first; }

	for (unsigned int i = 0;i < result.size();i++)
	{
		sp_Vector subResult;

		sp_Edits(result[i], subResult);
		sp_Known(subResult, candidates);
	}

	if (candidates.size() > 0) { return max_element(candidates.begin(), candidates.end(), sortBySecond)->first; }

	return "";
}

void SentenceKeywordRecog::sp_Known(sp_Vector& results, sp_Dictionary& candidates)
{
	sp_Dictionary::iterator end = sp_dict.end();

	for (unsigned int i = 0;i < results.size();i++)
	{
		sp_Dictionary::iterator value = sp_dict.find(results[i]);

		if (value != end) candidates[value->first] = value->second;
	}
}

void SentenceKeywordRecog::sp_Edits(const std::string& word, sp_Vector& result)
{
	for (string::size_type i = 0;i < word.size();    i++) result.push_back(word.substr(0, i)             + word.substr(i + 1)); //deletions
	for (string::size_type i = 0;i < word.size() - 1;i++) result.push_back(word.substr(0, i) + word[i+1] + word.substr(i + 2)); //transposition

	for (char j = 'a';j <= 'z';++j)
	{
		for (string::size_type i = 0;i < word.size();    i++) result.push_back(word.substr(0, i) + j + word.substr(i + 1)); //alterations
		for (string::size_type i = 0;i < word.size() + 1;i++) result.push_back(word.substr(0, i) + j + word.substr(i)    ); //insertion
	}
}