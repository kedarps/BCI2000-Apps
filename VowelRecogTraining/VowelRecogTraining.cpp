#include "VowelRecogTraining.h"

VowelRecogTraining::VowelRecogTraining(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags),
	WAVDIR("C:\\BCI2000\\v4369\\prog\\sounds\\VowelRecogTraining\\"),
	FILE_SETSINFO(WAVDIR),
	mRightTokenFullFile(WAVDIR),
	mWrongTokenFullFile(WAVDIR),
	
	iToken(0),
	myScore(0),

	isTokenFinished(false),
	isUserResponded(false),
	isRightTokenPlayed(false),
	isWrongTokenPlayed(false),
	isRightTokenFinished(false),
	isWrongTokenFinished(false),
	isFeedbackOver(false),
	isTaskDone(false),
	isReady(false),

	mColorPalette(new QPalette()),
	mTime(new QTime())
{
	ui.setupUi(this);
	
	// Connect buttons to respective slots
	// Grid 1
	connect(ui.G1_O1,SIGNAL(clicked()),this,SLOT(G1_O1_Clicked()));
	connect(ui.G1_O2,SIGNAL(clicked()),this,SLOT(G1_O2_Clicked()));
	// Grid 2
	connect(ui.G2_O1,SIGNAL(clicked()),this,SLOT(G2_O1_Clicked()));
	connect(ui.G2_O2,SIGNAL(clicked()),this,SLOT(G2_O2_Clicked()));
	connect(ui.G2_O3,SIGNAL(clicked()),this,SLOT(G2_O3_Clicked()));
	// Grid 3
	connect(ui.G3_O1,SIGNAL(clicked()),this,SLOT(G3_O1_Clicked()));
	connect(ui.G3_O2,SIGNAL(clicked()),this,SLOT(G3_O2_Clicked()));
	connect(ui.G3_O3,SIGNAL(clicked()),this,SLOT(G3_O3_Clicked()));
	connect(ui.G3_O4,SIGNAL(clicked()),this,SLOT(G3_O4_Clicked()));
	
	// Feedback Widget Grid
	connect(ui.Wrong_ok,SIGNAL(clicked()),this,SLOT(ok_Clicked()));
	connect(ui.Wrong_replay,SIGNAL(clicked()),this,SLOT(replay_Clicked()));

	connect(ui.ready,SIGNAL(clicked()),this,SLOT(readyClicked()));

	// Initialise number of sets for each speaker. Remember 0 indexing
	v_SpeakerNumSets.push_back(9);
	v_SpeakerNumSets.push_back(10);
	v_SpeakerNumSets.push_back(6);
	v_SpeakerNumSets.push_back(8);
	v_SpeakerNumSets.push_back(10);
	v_SpeakerNumSets.push_back(8);
	v_SpeakerNumSets.push_back(6);
	v_SpeakerNumSets.push_back(6);
	
	for(unsigned int idx = 0; idx < v_SpeakerNumSets.size(); idx++)
	{
		std::vector<int> v_SetNumsHere;
		for(int iSet = 1; iSet <= v_SpeakerNumSets[idx]; iSet++)
		{
			v_SetNumsHere.push_back(iSet);
		}
		v_SpeakerSetsToUse.push_back(v_SetNumsHere);
	}

	FILE_SETSINFO.append("Info_Sets.txt");
	ui.time->setHidden(true);
}

VowelRecogTraining::~VowelRecogTraining()
{
	delete mTime;
	delete mColorPalette;
}

void VowelRecogTraining::Initialize()
{
	ShowFeedbackWidgets(false);
	ShowGrids(false);
	ui.ready->setVisible(true);

	ui.status->move(ui.MainWidget->geometry().center().x() - ui.status->geometry().width()/2, 0);
	ui.ready->move(ui.MainWidget->geometry().center().x() - ui.ready->geometry().width()/2, ui.status->geometry().height());

	/***********Set Images**************/
	QPixmap mCorrectImg = *(new QPixmap("C:\\BCI2000\\v4369\\src\\custom\\VowelRecogTraining\\correct.png"));
	QPixmap mIncorrectImg = *(new QPixmap("C:\\BCI2000\\v4369\\src\\custom\\VowelRecogTraining\\incorrect.png"));

	ui.Right_CorrectImg->setPixmap(mCorrectImg);
	ui.Wrong_CorrectImg->setPixmap(mCorrectImg);
	ui.Wrong_IncorrectImg->setPixmap(mIncorrectImg);

	ui.Wrong_Incorrect_play->setPixmap(*(new QPixmap("C:\\BCI2000\\v4369\\src\\custom\\VowelRecogTraining\\play.png")));
	ui.Wrong_Correct_play->setPixmap(*(new QPixmap("C:\\BCI2000\\v4369\\src\\custom\\VowelRecogTraining\\play.png")));
	/***********************Main Task Init********************/
	
	//Shuffle random seed
	srand((unsigned)time(NULL));

	// These two for loops ensure that we do not use the same speaker set again
	int iSpeaker = 1;
	for(int idx = 0; idx < N_ITERS; idx++)
	{
		v_SpeakerIdx.push_back(iSpeaker);
		
		if(!(iSpeaker % v_SpeakerNumSets.size()))
			iSpeaker = 1;
		else
			iSpeaker++;
	}

	std::random_shuffle(v_SpeakerIdx.begin(),v_SpeakerIdx.end());

	for(int idx = 0; idx < N_ITERS; idx++)
	{
		int speakerHere = v_SpeakerIdx[idx];
		std::vector<int> setOptions = v_SpeakerSetsToUse[speakerHere-1];
		int setHere = setOptions[(int) rand()%setOptions.size()];
		v_SpeakerSetIdx.push_back(setHere);

		UpdateSpeakerSetsToUse(speakerHere-1,setHere);
	}

	for(int idx = 0; idx < N_ITERS; idx++)
	{
		// Initialize User Responses to 0
		v_UserResponses.push_back(0);
		v_isRight.push_back(0);
		
		// For given speaker set find how many options you have. We do a text search
		// First build what we want to search
		string txtToSearch = "Speaker";
		txtToSearch.append(std::to_string((long double) v_SpeakerIdx[idx]));
		txtToSearch.append("_Set");
		txtToSearch.append(std::to_string((long double) v_SpeakerSetIdx[idx]));
		
		// Do the search
		std::ifstream setInfoFile;
		setInfoFile.open(FILE_SETSINFO);
		
		unsigned int currLineIdx = 0;
		std::string currLine;

		std::vector<std::string> v_ThisTokenSet;

		while(std::getline(setInfoFile, currLine))
		{
			currLineIdx++;
			
			if (currLine.find(txtToSearch, 0) != std::string::npos)
			{
				std::string thisToken;
				
				// Now that you have the current line, extract the token names in given set
				std::string mAllTokensInThisSet = currLine.substr(currLine.find_first_of(" ")+1);
				std::istringstream iss(mAllTokensInThisSet);

				while(std::getline(iss, thisToken,' '))
				{
					v_ThisTokenSet.push_back(thisToken);
				}
				break;
			}
		}
		
		// Add Token Options to main vector
		v_TokenSetsIdx.push_back(v_ThisTokenSet);
		
		int thisSetOption = rand() % v_ThisTokenSet.size() + 1;
		v_OptionNosIdx.push_back(thisSetOption);

		std::string thisTokenName = txtToSearch;
		thisTokenName.append("_Option");
		thisTokenName.append(std::to_string((long double) thisSetOption));
		thisTokenName.append(".wav");

		v_TokenNamesIdx.push_back(thisTokenName);

		setInfoFile.close();
		setInfoFile.clear();
		v_ThisTokenSet.clear();
	}

	mTaskMode = m_WaitUserReady;
}

void VowelRecogTraining::UpdateSpeakerSetsToUse(int speakIdx, int setToDelete)
{
	std::vector<int> setOptions = v_SpeakerSetsToUse[speakIdx];

	for(unsigned int iVecIdx = 0; iVecIdx < setOptions.size(); iVecIdx++)
	{
		if(setOptions[iVecIdx] == setToDelete)
		{
			setOptions.erase(setOptions.begin()+iVecIdx);
			break;
		}
	}
	v_SpeakerSetsToUse[speakIdx] = setOptions;
}

void VowelRecogTraining::Start(void)
{
	//ui.status->move(ui.MainWidget->geometry().center().x() - ui.status->geometry().width()/2, 0);
	mTime->start();
}

void VowelRecogTraining::Process(void)
{
	QString startTxt, statusTxt;
	bool isRight;

	tym_Elapsed = mTime->elapsed();
	ui.time->display(tym_Elapsed);

	switch(mTaskMode)
	{
	case m_WaitUserReady:
		mTaskMode = isReady ? m_PreStart : m_WaitUserReady;
		break;
	case m_PreStart:
		ui.ready->setVisible(false);
		tym_ToWait = tym_Elapsed + TYM_PRESTART;
		mTaskMode = m_SleepToStart;
		break;
	case m_SleepToStart:
		if(((tym_ToWait - tym_Elapsed) > 2000) & ((tym_ToWait - tym_Elapsed) < 3000))
			startTxt.append("Starting in 3");
		else if(((tym_ToWait - tym_Elapsed) > 1000) & ((tym_ToWait - tym_Elapsed) < 2000))
			startTxt.append("Starting in 2");
		else if((tym_ToWait - tym_Elapsed) < 1000)
			startTxt.append("Starting in 1");

		ui.status->setText(startTxt);

		mTaskMode = ((tym_ToWait - tym_Elapsed) < 10) ? m_PlayToken : m_SleepToStart;
		break;
	case m_PlayToken:
		ShowFeedbackWidgets(false);
		if(isTaskDone)
		{
			statusTxt = "End of Training Run.\nYour score - ";
			statusTxt.append(QString::number(myScore));
			statusTxt.append(" out of ");
			statusTxt.append(QString::number(N_ITERS));
			ui.status->setText(statusTxt);
			break;
		}
		else
		{
			statusTxt.append("Trial ");
			statusTxt.append(QString::number(iToken+1));
			statusTxt.append(" of ");
			statusTxt.append(QString::number(N_ITERS));

			ui.Wrong_Correct_play->setVisible(false);
			ui.Wrong_Incorrect_play->setVisible(false);

			PrepUIForToken();
			
			ui.status->setText(statusTxt);
			mTaskMode = m_SleepToEndToken;
			break;
		}
	case m_SleepToEndToken:
		mTaskMode = isTokenFinished ? m_SleepTillUserResponse : m_SleepToEndToken;
		break;
	case m_SleepTillUserResponse:
		mTaskMode = isUserResponded ? m_GiveFeedback : m_SleepTillUserResponse;
		break;
	case m_GiveFeedback:
		isRight = GiveFeedback();		
		if(isRight)
		{
			tym_ToWait = tym_Elapsed + TYM_RIGHTFEEDBACK;
			mTaskMode = m_SleepToEndRightFeedback;
		}
		else
			mTaskMode = m_SleepToEndWrongFeedback;
		break;
	case m_SleepToEndRightFeedback:
		mTaskMode = ((tym_ToWait - tym_Elapsed) < 10) ? m_PrepNextToken : m_SleepToEndRightFeedback;
		break;
	case m_SleepToEndWrongFeedback:
		mTaskMode = isFeedbackOver ? m_PrepNextToken : m_SleepToEndWrongFeedback;
		if(!isWrongTokenFinished && !isRightTokenFinished)
		{
			ui.Wrong_Incorrect_play->setVisible(true);
			ui.Wrong_Correct_play->setVisible(false);
		}
		if(isWrongTokenFinished && !isRightTokenFinished)
		{
			ui.Wrong_Incorrect_play->setVisible(false);
			ui.Wrong_Correct_play->setVisible(true);
		}	
		if(!isRightTokenFinished || !isWrongTokenFinished)
		{
			ui.Wrong_ok->setDisabled(true);
			ui.Wrong_replay->setDisabled(true);
		}
		if(isRightTokenFinished && isWrongTokenFinished)
		{
			ui.Wrong_ok->setDisabled(false);
			ui.Wrong_replay->setDisabled(false);

			ui.Wrong_Correct_play->setVisible(false);
		}
		break;
	case m_PrepNextToken:
		iToken++;
		isTokenFinished = false;
		isUserResponded = false;
		isFeedbackOver = false;
		isRightTokenPlayed = false;
		isWrongTokenPlayed = false;
		isRightTokenFinished = false;
		isWrongTokenFinished = false;
	
		if(iToken == N_ITERS)
			isTaskDone = true;
		
		mRightTokenFullFile = WAVDIR;
		mWrongTokenFullFile = WAVDIR;

		mTaskMode = m_PlayToken;
		break;
	}
}

// This function displays respective button grid with button names
void VowelRecogTraining::PrepUIForToken(void)
{
	ShowGrids(false);

	std::vector<std::string> thisTokenSet = v_TokenSetsIdx[iToken];

	mTokenName = thisTokenSet[v_OptionNosIdx[iToken]-1];

	switch(thisTokenSet.size())
	{
	case 2:
		ShowGrids(1,true);
		ui.G1_O1->setText(QString::fromStdString(thisTokenSet[0]));
		ui.G1_O2->setText(QString::fromStdString(thisTokenSet[1]));
		break;
	case 3:
		ShowGrids(2,true);
		ui.G2_O1->setText(QString::fromStdString(thisTokenSet[0]));
		ui.G2_O2->setText(QString::fromStdString(thisTokenSet[1]));
		ui.G2_O3->setText(QString::fromStdString(thisTokenSet[2]));
		break;
	case 4:
		ShowGrids(3,true);
		ui.G3_O1->setText(QString::fromStdString(thisTokenSet[0]));
		ui.G3_O2->setText(QString::fromStdString(thisTokenSet[1]));
		ui.G3_O3->setText(QString::fromStdString(thisTokenSet[2]));
		ui.G3_O4->setText(QString::fromStdString(thisTokenSet[3]));
		break;
	}
}

// Returns true if answer was Right else returns false
bool VowelRecogTraining::GiveFeedback(void)
{
	// Compare Option number and user response to find if answer is right
	ShowGrids(false);

	if(v_OptionNosIdx[iToken] == v_UserResponses[iToken])
	{
		ShowFeedbackWidgets((int) 1);
		v_isRight[iToken] = 1;
		myScore++;
		return true;
	}
	else
	{
		ShowFeedbackWidgets((int) 0);
		std::vector<std::string> thisTokenSet = v_TokenSetsIdx[iToken];
		
		ui.Wrong_CorrectToken->setText(QString::fromStdString(thisTokenSet[v_OptionNosIdx[iToken]-1]));
		ui.Wrong_IncorrectToken->setText(QString::fromStdString(thisTokenSet[v_UserResponses[iToken]-1]));

		std::string tokenFileName = "Speaker";
		tokenFileName.append(std::to_string((long double) v_SpeakerIdx[iToken]));
		tokenFileName.append("_Set");
		tokenFileName.append(std::to_string((long double) v_SpeakerSetIdx[iToken]));
		tokenFileName.append("_Option");

		mRightTokenFullFile.append(tokenFileName);
		mRightTokenFullFile.append(std::to_string((long double) v_OptionNosIdx[iToken]));
		mRightTokenFullFile.append(".wav");

		mWrongTokenFullFile.append(tokenFileName);
		mWrongTokenFullFile.append(std::to_string((long double) v_UserResponses[iToken]));
		mWrongTokenFullFile.append(".wav");

		return false;
	}
}

void VowelRecogTraining::ShowGrids(int gridIdx, bool show)
{
	switch(gridIdx)
	{
	case 1:
		ui.Grid1_Widget->move(ui.MainWidget->geometry().center().x() - ui.Grid1_Widget->geometry().width()/2,ui.MainWidget->geometry().center().y() - ui.Grid1_Widget->geometry().height()/2);
		ui.Grid1_Widget->setVisible(show);
		break;
	case 2:
		ui.Grid2_Widget->move(ui.MainWidget->geometry().center().x() - ui.Grid2_Widget->geometry().width()/2,ui.MainWidget->geometry().center().y() - ui.Grid2_Widget->geometry().height()/2);
		ui.Grid2_Widget->setVisible(show);
		break;
	case 3:
		ui.Grid3_Widget->move(ui.MainWidget->geometry().center().x() - ui.Grid3_Widget->geometry().width()/2,ui.MainWidget->geometry().center().y() - ui.Grid3_Widget->geometry().height()/2);
		ui.Grid3_Widget->setVisible(show);
		break;
	}
}

void VowelRecogTraining::ShowGrids(bool show)
{
	ui.Grid1_Widget->setVisible(show);
	ui.Grid2_Widget->setVisible(show);
	ui.Grid3_Widget->setVisible(show);
}

void VowelRecogTraining::ShowFeedbackWidgets(int isRight)
{
	switch(isRight)
	{
	case 0: // Show Wrong
		ui.RightWidget->setVisible(false);
		ui.WrongWidget->setVisible(true);
		ui.WrongWidget->move(ui.MainWidget->geometry().center().x() - ui.WrongWidget->geometry().width()/2,ui.MainWidget->geometry().center().y() - ui.WrongWidget->geometry().height()/2);
		break;
	case 1: // Show Right
		ui.RightWidget->setVisible(true);
		ui.WrongWidget->setVisible(false);
		ui.RightWidget->move(ui.MainWidget->geometry().center().x() - ui.RightWidget->geometry().width()/2,ui.MainWidget->geometry().center().y() - ui.RightWidget->geometry().height()/2);
		break;
	}
}

void VowelRecogTraining::ShowFeedbackWidgets(bool hideAll)
{
	ui.RightWidget->setVisible(hideAll);
	ui.WrongWidget->setVisible(hideAll);
}

void VowelRecogTraining::HideAllWidgets(bool hideAll)
{
	ui.Grid1_Widget->setVisible(hideAll);
	ui.Grid2_Widget->setVisible(hideAll);
	ui.Grid3_Widget->setVisible(hideAll);

	ui.RightWidget->setVisible(hideAll);
	ui.WrongWidget->setVisible(hideAll);
}

void VowelRecogTraining::G1_O1_Clicked()
{
	if((mTaskMode == m_SleepToEndToken) || (mTaskMode == m_SleepTillUserResponse))
	{
		v_UserResponses[iToken] = 1;
		isUserResponded = true;
	}
}

void VowelRecogTraining::G1_O2_Clicked()
{
	if((mTaskMode == m_SleepToEndToken) || (mTaskMode == m_SleepTillUserResponse))
	{
		v_UserResponses[iToken] = 2;
		isUserResponded = true;
	}
}

void VowelRecogTraining::G2_O1_Clicked()
{
	if((mTaskMode == m_SleepToEndToken) || (mTaskMode == m_SleepTillUserResponse))
	{
		v_UserResponses[iToken] = 1;
		isUserResponded = true;
	}
}

void VowelRecogTraining::G2_O2_Clicked()
{
	if((mTaskMode == m_SleepToEndToken) || (mTaskMode == m_SleepTillUserResponse))
	{
		v_UserResponses[iToken] = 2;
		isUserResponded = true;
	}
}

void VowelRecogTraining::G2_O3_Clicked()
{
	if((mTaskMode == m_SleepToEndToken) || (mTaskMode == m_SleepTillUserResponse))
	{
		v_UserResponses[iToken] = 3;
		isUserResponded = true;
	}
}

void VowelRecogTraining::G3_O1_Clicked()
{
	if((mTaskMode == m_SleepToEndToken) || (mTaskMode == m_SleepTillUserResponse))
	{
		v_UserResponses[iToken] = 1;
		isUserResponded = true;
	}	
}

void VowelRecogTraining::G3_O2_Clicked()
{
	if((mTaskMode == m_SleepToEndToken) || (mTaskMode == m_SleepTillUserResponse))
	{
		v_UserResponses[iToken] = 2;
		isUserResponded = true;
	}
}

void VowelRecogTraining::G3_O3_Clicked()
{
	if((mTaskMode == m_SleepToEndToken) || (mTaskMode == m_SleepTillUserResponse))
	{
		v_UserResponses[iToken] = 3;
		isUserResponded = true;
	}
}

void VowelRecogTraining::G3_O4_Clicked()
{
	if((mTaskMode == m_SleepToEndToken) || (mTaskMode == m_SleepTillUserResponse))
	{
		v_UserResponses[iToken] = 4;
		isUserResponded = true;
	}
}

void VowelRecogTraining::ok_Clicked()
{
	isFeedbackOver = true;
}

void VowelRecogTraining::replay_Clicked()
{
	isRightTokenFinished = false;
	isWrongTokenFinished = false;

	isRightTokenPlayed = false;
	isWrongTokenPlayed = false;
}

void VowelRecogTraining::readyClicked()
{
	isReady = true;
}
