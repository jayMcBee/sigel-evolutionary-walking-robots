#ifndef MT_CONTROLLER_H
#define MT_CONTROLLER_H

#include "compat/q2compat.h"
class MT_MainWindow;
#include "MT_Control/MT_Substitute.h"
#include "MT_GPSystem/MT_GPManager.h"

#include <qtimer.h>
#include <qstring.h>
#include <qtextstream.h>
#ifdef _WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#endif

#define NOMETA_SUBST	 0
#define EVALUATOR_SUBST  1
#define CLASSIFIER_SUBST 2

namespace SIGEL_GP {
	class SIG_GPExperiment;
};

class MT_Controller : public QObject
{
	Q_OBJECT

#ifdef _WINDOWS	
	friend DWORD WINAPI StartMetaEvolution(LPVOID inRawSubst);
#else	
	friend void StartMetaEvolution(void *inRawSubst);
#endif

public:
	MT_Controller(SIGEL_GP::SIG_GPExperiment &exp);
	~MT_Controller();

public:
	bool startWOSigel;

	void configureSystem();
	bool useMeta(bool state);
	bool switchSystem(int wantedSystem);
	bool startEvolution();
	void startTimedEvolution(int minutes);
	bool readFromFile(QString fileName);
	void writeToFile(QTextStream &file, QString name, int autoSave);
	bool IsEnabled();
	int UsedSystem();
	MT_Substitute* getFitnessTrainer();
	MT_Substitute* getClassifier();


private:
	bool withGUI;
	int selectedSystem;
	int autoSaveCnt;
	QTimer *evolTimer;
	MT_GPManager *gpManager;
	MT_MainWindow *mainWindow;
	MT_Substitute *substitution;
	SIGEL_GP::SIG_GPExperiment &sigExp;
	QObject *sigMainWindow;
	Q2Array<unsigned int> correctEst;
	Q2Array<unsigned int> metaEst;

#ifdef _WINDOWS
	HANDLE meta_thread;
#else
	pthread_t meta_thread;
#endif

	int usedSystem;
	bool metaOn;

	QFile confFile;

	QTextStream confStrm;
	QTextStream *cacheStrm;
	QString defConfFileName;
	QString cacheString;

	QString saveName;
	QString baseName;
	subst_cache substCache;

	bool createGPSystem();
	bool saveSystem(QString sigExpName);
	void callMetaEvolutionLoop();
	void callMetaEvolutionLoopWOSigel();

	void loadCache(QTextStream &File);
	void saveCache(QTextStream &File);
	void createCacheStrm();

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