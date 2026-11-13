/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler

  This file is part of Sigel.

  Sigel is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Sigel is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Sigel; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef MT_CONTROLLER_H
#define MT_CONTROLLER_H

#include <QList>
#include <QString>
#include <QTextStream>
#include <QFile>
class MT_MainWindow;
class QWidget;
#include "MT_Control/MT_EstimationState.h"
#include "MT_Control/MT_Substitute.h"
#include "MT_GPSystem/MT_GPManager.h"

#include <qtimer.h>
#include <qstring.h>
#include <qtextstream.h>
#include <pthread.h>

#include "MT_Control/MT_SubstituteKind.h"

namespace SIGEL_GP {

	class SIG_GPMetaGPExperiment;

}

class MT_Controller : public QObject
{
	Q_OBJECT

	friend void StartMetaEvolution(void *inRawSubst);

public:
	MT_Controller(SIGEL_GP::SIG_GPMetaGPExperiment &exp);
	~MT_Controller() override;

public:
	bool startWOSigel;

	void configureSystem(QWidget *owner);
	bool useMeta(bool state);
	bool switchSystem(MT_SubstituteKind wantedSystem);
	bool startEvolution();
	void startTimedEvolution(int minutes);
	bool readFromFile(QString fileName);
	bool writeToFile(QTextStream &file, QString name);
	bool IsEnabled();
	MT_SubstituteKind UsedSystem();
	MT_Substitute* getFitnessTrainer();
	MT_Substitute* getClassifier();

private:
	bool withGUI;
	QTimer *evolTimer;
	MT_GPManager *gpManager;
	MT_MainWindow *mainWindow;
	MT_Substitute *substitution;
	SIGEL_GP::SIG_GPMetaGPExperiment &sigExp;
	QObject *sigMainWindow;
	QList<unsigned int> correctEst;
	QList<unsigned int> metaEst;

	pthread_t meta_thread;

	MT_SubstituteKind usedSystem;
	bool metaOn;

	QFile confFile;

	QTextStream confStrm;
	QString defConfFileName;
	QString estimationSettingsText;
	QTextStream estimationSettingsStream;

	QString saveName;
	MT_EstimationState estimationState;

	bool createGPSystem();
	bool saveSystem(QString sigExpName);
	void callMetaEvolutionLoop();
	void callMetaEvolutionLoopWOSigel();

	void loadEstimationState(QTextStream &File);
	void saveEstimationState(QTextStream &File);
	void writeEstimationSettings();

public slots:
	void stopEvolution();
private slots:
	void slotLoadDefault();
	void slotLoadSetup();
	void slotSaveSetup();
	void startSingleEvolution();
	void slotEvolutionRunning(bool running);
};

#endif