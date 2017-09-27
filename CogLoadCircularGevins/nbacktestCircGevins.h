#ifndef NBACKTESTCIRCGEVINS_H
#define NBACKTESTCIRCGEVINS_H

#include <QtGui\QMainWindow>
#include <QtGui\QWidget>
#include <QtGui\QPalette>
#include <QtGui\QPainter>
#include <QtGui\QColor>
#include <QtGui\QPen>

#include <QtCore\Qtime>
#include <QtCore\QPoint>
#include <QtCore\QRect>
#include <QtCore\QLineF>
#include <QtCore\Qtimer>
#include <QtCore\QString>
#include <QtCore\QVector>
#include <QDesktopWidget>

#include <QKeyEvent>
#include <QPaintEvent>

#include "ui_nbacktestCircGevins.h"

#include <iostream>
#include <time.h>
#include <vector>

using namespace std;

class nBackTestCircGevins : public QMainWindow
{
	Q_OBJECT

private:
	static const int N_ITERS = 50;
	static const int N_BACK = 3;

	Ui::nBackTestCircGevinsClass ui;

	int taskIdx;

	std::vector<int>charSet,
		charIdx,
		charPosIdx,
		matchMode,
		userResp;

	QVector<QRect>charPos;
	QVector<QLineF>taskWheel;
	double taskWheelRad;
	int fixPtRad;

	int iTaskIter,
		initIters,
		nIters2Use;

	QTime *myTime;
	int	idleTym,
		fixPtDispTym,
		charDispTym,
		w8inTym;

	QKeyEvent *keyPress;
	QPalette *myColorPalette;

public:
	nBackTestCircGevins(QWidget *parent = 0, Qt::WFlags flags = 0);
	~nBackTestCircGevins();
	void InitTask(int);
	void StartTask(void);
	void ProcessTask(void);

	bool isReady, isTaskInProgress, isDone;
	int myScore, taskDispMode;

protected:
	void keyReleaseEvent(QKeyEvent *);
	void paintEvent(QPaintEvent *);
};

#endif // NBACKTESTCIRCGEVINS_H
