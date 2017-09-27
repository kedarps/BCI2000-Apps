#include "nbacktestCircGevins.h"

nBackTestCircGevins::nBackTestCircGevins(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags),
	charSet(),
	charIdx(), 
	charPosIdx(),
	matchMode(), 
	userResp(),

	taskWheel(),
	taskWheelRad(225),
	fixPtRad(15),

	isReady(false),
	isDone(false),
	isTaskInProgress(false),

	idleTym(2750),
	fixPtDispTym(1500),
	charDispTym(250),
	w8inTym(0),
	
	/*idleTym(750),
	fixPtDispTym(500),
	charDispTym(250),
	w8inTym(0),*/

	iTaskIter(0),
	myScore(0),
	taskDispMode(0),

	myColorPalette(new QPalette()),
	myTime(new QTime())
{
	ui.setupUi(this);
	ui.taskLetterLabel->setHidden(true);
}

nBackTestCircGevins::~nBackTestCircGevins()
{
	delete myTime;
	delete myColorPalette;
}

void nBackTestCircGevins::InitTask(int taskIdx)
{
	QString statTxt, instrTxt;
	statTxt.append("Initialized ");
	instrTxt.append("Instructions: At Each Trial determine if ");

	/***********Initialise geometry for the task**************/
	//Window Center and current point
	QPoint winCntr; 

	winCntr.setX(ui.centralWidget->geometry().width()/2);
	winCntr.setY(ui.centralWidget->geometry().height()/2);

	for(double angle = 0; angle < 2*M_PI; angle += M_PI/6)
	{
		QPoint currPoint, charLocPos;
		QRect currGeom;

		//Get Current-Point using parametric equation of circle
		currPoint.setX(taskWheelRad*std::cos(angle));
		currPoint.setY(taskWheelRad*std::sin(angle));

		//Shift the point to centre
		currPoint += winCntr;

		//Corresponding Task-Wheel spoke is lien between Current-Point and Centre of Window
		taskWheel.push_back(QLineF(winCntr,currPoint));

		//Put Character-Position such that Current-Point is at center of the character
		charLocPos.setX(currPoint.rx() - ui.taskLetterLabel->geometry().width()/2 + 50*std::cos(angle));
		charLocPos.setY(currPoint.ry() - ui.taskLetterLabel->geometry().height()/2 + 50*std::sin(angle));

		currGeom.setTopLeft(charLocPos);
		currGeom.setWidth(ui.taskLetterLabel->geometry().width());
		currGeom.setHeight(ui.taskLetterLabel->geometry().height());

		charPos.push_back(currGeom);
	}

	//Put the Status Label in the Center
	/*int xStart, yStart;
	xStart = (ui.centralWidget->geometry().width() - ui.statusLabel->geometry().width())/2;
	yStart = (ui.centralWidget->geometry().height() - ui.statusLabel->geometry().height())/2;
	ui.statusLabel->setGeometry(QRect(xStart,yStart,ui.statusLabel->geometry().width(),ui.statusLabel->geometry().height()));*/

	/****************Main Task Init*****************/
	//Shuffle random seed
	srand((unsigned)time(NULL));

	//Get unique set of characters for the task
	for(int iCh = 0; iCh < 12; iCh++) 
	{
		int tmpNum = 0;

		if(iCh == 0)
			//At First Iter we just add a random value
			tmpNum = rand()%26 + 65;
		else
		{
			//At Subsequent Iters we check if charSet already contains the random value
			do tmpNum = rand()%26 + 65;
			while(std::find(charSet.begin(),charSet.end(),tmpNum) != charSet.end());
		}
		charSet.push_back(tmpNum);
	}

	//We add N_ITERS based on 3-Back or First-Back
	int idx2cmp;
	if((taskIdx == 1) || (taskIdx == 3)) 
	{
		nIters2Use = N_ITERS + N_BACK;
		initIters = N_BACK;
		statTxt.append(QString::number(N_BACK));
		statTxt.append("-Back ");
	}
	else 
	{
		nIters2Use = N_ITERS + 1;
		initIters = 1;
		statTxt.append("First Back ");
	}

	if(taskIdx <= 2)
	{
		statTxt.append("Verbal");
		instrTxt.append("'Character' matches the character flashed ");
	}
	else
	{
		statTxt.append("Spatial");
		instrTxt.append("'Position' of character matches the position of character flashed ");
	}
	
	if((taskIdx == 1) || (taskIdx == 3)) 
	{
		instrTxt.append(QString::number(N_BACK));
		instrTxt.append("-Trials Before. ");
	}
	else
		instrTxt.append("at the First Trial. ");

	instrTxt.append("\nIf match found press 'Space'");
	for(int iIter = 0;iIter < nIters2Use;iIter++)
	{
		int tmpChar, tmpPos;
		if(iIter < initIters)
		{
			if(iIter == 0)
			{
				//At 0th iIter choose Random Values
				tmpChar = rand() % 12;
				tmpPos = rand() % 12;
			}
			else
			{
				//Between 0 and initIters make sure we get unique random values
				do tmpChar = rand() % 12; while(tmpChar == charIdx[iIter-1]);
				do tmpPos = rand() % 4; while(tmpPos == charPosIdx[iIter-1]);
			}
			matchMode.push_back(2);
			userResp.push_back(2);
		}
		else
		{
			//Choose which iteration to compare based on task type
			idx2cmp = ((taskIdx == 1) || (taskIdx == 3)) ? (iIter - N_BACK) : 0;

			//Randomly choose match or no match. Match on 30% of Trials
			int randMode = (((double) rand()/(RAND_MAX)) < 0.3) ? 1 : 0;
			matchMode.push_back(randMode);
			userResp.push_back(0);
			//userResp.push_back(randMode);

			//Match, matchMode == 1 
			if(matchMode[iIter])
			{
				//Verbal Match
				if((taskIdx == 1) || (taskIdx == 2))
				{
					//Get char from idx2cmp index
					tmpChar = charIdx[idx2cmp];
					tmpPos = rand() % 12; 
				}
				//Spatial Match
				else
				{
					//Get Position from idx2cmp index
					tmpPos = charPosIdx[idx2cmp];
					tmpChar = rand() % 12;
				}
			}
			//Don't Match, matchMode == 0
			else
			{
				//If theres no match get random make sure we do not match with idx2cmp
				do tmpChar = rand() % 12; while(tmpChar == charIdx[idx2cmp]);
				do tmpPos = rand() % 12; while(tmpPos == charPosIdx[idx2cmp]);
			}
		}
		charIdx.push_back(tmpChar);
		charPosIdx.push_back(tmpPos);
	}
	isReady = true;
	ui.statusLabel->setText(statTxt);
	ui.instrLabel->setText(instrTxt);

	ui.timeDisplay->setHidden(true);
}

void nBackTestCircGevins::StartTask()
{
	ui.statusLabel->setText(" ");
	ui.instrLabel->setText(" ");
	myTime->start();
}

void nBackTestCircGevins::ProcessTask()
{
	isTaskInProgress = (iTaskIter >= initIters) ? true : false;

	switch(taskDispMode)
	{
		//This shows the characters as described by Gevins et al. No feedback. 
		//Each trial is 4.5s long. 
		//0-2.75 --> Idle (2.75s)
		//2.75-4.25 --> Center Fixation Point (1.5s)
		//4.25-4.5 --> Character (0.250s)
	case 0://Idle Mode
		if(iTaskIter >= nIters2Use)
		{
			isDone = true;
			isTaskInProgress = false;
			ui.taskLetterLabel->setHidden(true);

			QString scoreDisp;
			scoreDisp = "TaskOver, Your Score ";
			scoreDisp.append(QString::number(myScore));
			scoreDisp.append(" Out of ");
			scoreDisp.append(QString::number(nIters2Use - initIters));
			ui.statusLabel->setText(scoreDisp);
			break;
		}
		w8inTym = myTime->elapsed() + idleTym;
		taskDispMode = 1;
		break;
	case 1://Idle Wait
		taskDispMode = ((w8inTym - myTime->elapsed()) < 50) ? 2 : 1;
		break;
	case 2://Fixation Point Display
		w8inTym = myTime->elapsed() + fixPtDispTym;

		if (isTaskInProgress && ((userResp[iTaskIter-1] == matchMode[iTaskIter-1])))
			myScore++;

		/*if(iTaskIter >= nIters2Use-1)
		{
			isDone = true;
			isTaskInProgress = false;
			ui.taskLetterLabel->setHidden(true);

			QString scoreDisp;
			scoreDisp = "TaskOver, Your Score ";
			scoreDisp.append(QString::number(myScore));
			scoreDisp.append(" Out of ");
			scoreDisp.append(QString::number(nIters2Use - initIters));
			ui.statusLabel->setText(scoreDisp);
			break;
		}*/
		taskDispMode = 3;
		break;
	case 3://Fixation Point Wait
		taskDispMode = ((w8inTym - myTime->elapsed()) < 50) ? 4 : 3;
		break;
	case 4://Character Display
		ui.taskLetterLabel->setGeometry(charPos[charPosIdx[iTaskIter]]);
		ui.taskLetterLabel->setText(QString(charSet[charIdx[iTaskIter]]));
		myColorPalette->setColor(QPalette::WindowText,Qt::blue);
		ui.taskLetterLabel->setPalette(*myColorPalette);
		ui.taskLetterLabel->setHidden(false);
		w8inTym = myTime->elapsed() + charDispTym;
		taskDispMode = 5;
		break;
	case 5:
		if((w8inTym - myTime->elapsed()) < 50)
		{
			ui.taskLetterLabel->setHidden(true);
			taskDispMode = 0;
			iTaskIter++;
		}
		break;
	}
	ui.timeDisplay->display(myTime->elapsed());

	/*QString statTxt;
	statTxt.append("iTaskIter = ");
	statTxt.append(QString::number(iTaskIter));
	statTxt.append("; myScore = ");
	statTxt.append(QString::number(myScore));
	statTxt.append("; isTaskInProgress = ");
	statTxt.append(QString::number(isTaskInProgress));

	ui.statusLabel->setText(statTxt);*/
}

void nBackTestCircGevins::keyReleaseEvent(QKeyEvent *event)
{
	userResp[iTaskIter-1] = ((event->key() == Qt::Key_Space) && (isTaskInProgress)) ? 1 : 0;
}

void nBackTestCircGevins::paintEvent(QPaintEvent *event)
{
	if(isReady)
	{
		QPainter myPainter(this);

		if((taskDispMode == 4) || (taskDispMode == 5))
		{
			for(int iLine = 0;iLine < taskWheel.size();iLine++)
			{
				(iLine == charPosIdx[iTaskIter]) ? myPainter.setPen(QPen(Qt::black,2,Qt::SolidLine)) : myPainter.setPen(QPen(Qt::white,2,Qt::SolidLine));
				myPainter.drawLine(taskWheel[iLine]);
			}
		}
		else
		{
			myPainter.setPen(QPen(Qt::white,2,Qt::SolidLine));
			myPainter.drawLines(taskWheel);
		}

		myPainter.setPen(QPen(Qt::white,2,Qt::SolidLine));
		((taskDispMode == 2) || (taskDispMode == 3))  ? myPainter.setBrush(QBrush(Qt::white)) : myPainter.setBrush(QBrush(Qt::black));
		myPainter.drawEllipse(QPoint(ui.centralWidget->geometry().width()/2,ui.centralWidget->geometry().height()/2),fixPtRad,fixPtRad);
		
		update();
	}
}