#include <QCoreApplication>   // qApp; was reached via SIG_Program.h before A7
#include "SIGEL_Tools/SIG_IO.h"
#include "MT_GUI/MT_MainWindow.h"
#include "MT_Control/MT_Controller.h"
#include "MT_Control/MT_Classifier.h"
#include "MT_Control/MT_Evaluator.h"
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qbuffer.h>
#include <pvm3.h>

extern bool guiEnabled;

/***
 * this function is just used as the entry point of the meta thread
 ***/
#ifdef _WINDOWS
DWORD WINAPI StartMetaEvolution(LPVOID inRawSubst)
{
	MT_Controller *th_contr = (MT_Controller *)inRawSubst;
	if(th_contr->startWOSigel)
		th_contr->callMetaEvolutionLoopWOSigel();
	else
		th_contr->callMetaEvolutionLoop();
	return 0;
}
#else
void StartMetaEvolution(void *inRawSubst)
{
	MT_Controller *th_contr = (MT_Controller *)inRawSubst;
	if(th_contr->startWOSigel)
		th_contr->callMetaEvolutionLoopWOSigel();
	else
		th_contr->callMetaEvolutionLoop();
}
#endif


MT_Controller::MT_Controller(SIGEL_GP::SIG_GPExperiment &exp)
 : QObject(), gpManager(0), mainWindow(0), substitution(0), evolTimer(0), sigExp(exp), autoSaveCnt(0)
{
	startWOSigel = false;
	withGUI = true;
	cacheStrm = 0;
	saveName = QString();
	usedSystem = NOMETA_SUBST;
	selectedSystem = EVALUATOR_SUBST;
	metaOn = false;

	substCache.inUse = false;
	substCache.genNumber = 0;
	substCache.numCorrectEst = 0;
	substCache.numMetaEst = 0;
	substCache.refreshInt = 0;
	substCache.strategy = -1;
	substCache.tolerance = 0;

#ifdef _WINDOWS
	defConfFileName = ::getenv("SIGEL_ROOT");
#else
	defConfFileName = std::getenv("SIGEL_ROOT");
#endif
	defConfFileName.append("/stdConf.mt");
}

MT_Controller::~MT_Controller()
{
	if(confFile.isOpen()){
		confFile.close();
	}
	delete mainWindow;
	delete gpManager;
	delete substitution;
}

/***
 * start evolution without sigel 
 * called by the configuration window's start button
 ***/
void MT_Controller::startSingleEvolution()
{
	startWOSigel = true;

#ifdef _WINDOWS
		meta_thread = CreateThread(NULL, 0, &StartMetaEvolution, this, 0, 0);
//		SetThreadPriority(meta_thread, THREAD_PRIORITY_BELOW_NORMAL);
#else
		pthread_create(&meta_thread, NULL, (void*(*)(void*)) &StartMetaEvolution, (void*) this);
#endif

}

/***
 * starts the evolution with a timer that stops it if the given duration runs down
 * is called by the main() routine (if started with the appropiate options)
 ***/
void MT_Controller::startTimedEvolution(int minutes)
{
	withGUI = false;

	if(!evolTimer)
		evolTimer = new QTimer(this);

	startWOSigel = true;

	QObject::connect(evolTimer, SIGNAL(timeout()), SLOT(stopEvolution()));
	createGPSystem();
	evolTimer->setSingleShot(true);
	evolTimer->start(minutes * 60000);

	QObject::connect(gpManager, SIGNAL(metaEvolutionRunning(bool)), this, SLOT(slotEvolutionRunning(bool)));
#ifdef _WINDOWS
		meta_thread = CreateThread(NULL, 0, &StartMetaEvolution, this, 0, 0);
//		SetThreadPriority(meta_thread, THREAD_PRIORITY_BELOW_NORMAL);
#else
		pthread_create(&meta_thread, NULL, (void*(*)(void*)) &StartMetaEvolution, (void*) this);
#endif
}

void MT_Controller::slotEvolutionRunning(bool running)
{
	if(!running){
		if(startWOSigel && !withGUI){
			QObject::disconnect(gpManager, SIGNAL(metaEvolutionRunning(bool)), this, SLOT(slotEvolutionRunning(bool)));
			qApp->exit(0);
		}
	}
}

/***
 * starts the evolution
 * is called by SIGEL if the SIGEL evolution starts
 ***/
bool MT_Controller::startEvolution()
{
	startWOSigel = false;

	if(metaOn){

		if(!createGPSystem())	// create the gp system if not created so far
			return false;

		// make sure there's an actually correct substituter
		if(!substitution){
			createCacheStrm();

			switch(usedSystem){
			case EVALUATOR_SUBST :
				substitution = new MT_Evaluator(sigExp, *cacheStrm);
				break;
			case CLASSIFIER_SUBST :
				substitution = new MT_Classifier(*cacheStrm);
				break;
			}
			if(!substitution){
				if(guiEnabled)
					SIGEL_Tools::SIG_IO::cerr << "Creating Meta GP-System: couldn't create a substituter." << Qt::endl;
				return false;
			}
		}

		// create a thread for the meta-evolution
#ifdef _WINDOWS
		meta_thread = CreateThread(NULL, 0, &StartMetaEvolution, this, 0, 0);
//		SetThreadPriority(meta_thread, THREAD_PRIORITY_BELOW_NORMAL);
//		SetThreadPriorityBoost(meta_thread, true);
#else
		pthread_create(&meta_thread, NULL, (void*(*)(void*)) &StartMetaEvolution, (void*) this);
#endif
	}

	return true;
}

void MT_Controller::callMetaEvolutionLoop()
{
	gpManager->startEvolution(substitution);
}

void MT_Controller::callMetaEvolutionLoopWOSigel()
{
	gpManager->startEvolution( 0 );
}

/***
 * stops the evolution
 * is called by SIGEL if it hits a termination criterion
 ***/
void MT_Controller::stopEvolution()
{
	if(IsEnabled() && gpManager)
		gpManager->stopEvolution();
}

/***
 * switch between the two meta-systems
 * called from sig_mainwindow
 ***/
bool MT_Controller::switchSystem(int wantedSystem)
{

	// the wanted system equals the used system -> nothing to do so leave here
	if(usedSystem == wantedSystem)
		return true;

	// if we have constructed a substituter or a gpmanager yet, we have to destroy them
	if(gpManager || substCache.inUse || substitution){

		if(QMessageBox::warning(0, "Switching the meta system", 
			"Switching the system requires deleting the actual\n"
			"gp-system. Do you want to delete it?",
			"Yes", "No", 0, 1, 1) == 0){

			if(mainWindow){
				QObject::disconnect(mainWindow->mtStartEvolutionAction, SIGNAL( triggered() ), this, SLOT( startSingleEvolution() ));
				QObject::disconnect(mainWindow->mtStopEvolutionAction, SIGNAL( triggered() ), this, SLOT( stopEvolution() ));
				QObject::disconnect(mainWindow->mtDefaultAction, SIGNAL( triggered() ), this, SLOT(slotLoadDefault() ) );
				QObject::disconnect(mainWindow->mtLoadAction, SIGNAL( triggered() ), this, SLOT( slotLoadSetup() ) );
				QObject::disconnect(mainWindow->mtSaveAction, SIGNAL( triggered() ), this, SLOT( slotSaveSetup() ));
				QObject::disconnect(mainWindow->evolTimer, SIGNAL( timeout() ), this, SLOT( stopEvolution() ));
			}

			delete mainWindow;
			delete gpManager;
			delete substitution;
			substCache.inUse = false;
			mainWindow = 0;
			gpManager = 0;
			substitution = 0;
			confStrm.setDevice(nullptr);
			if(confFile.isOpen()) confFile.close();
		} else
			return false;
	}

	// now really switch the systems
	selectedSystem = wantedSystem;
	usedSystem = wantedSystem;

	// that's it; new systems are created by the time they are needed
	return true;
}

/***
 * turn the use of the meta gp system on/off
 ***/
bool MT_Controller::useMeta(bool state)
{
	if(metaOn == state)
		return false;

	if(state == false){		// the meta-system isn't needed anymore - therefore delete it?
		QString proposedName = saveName;
		if(guiEnabled){
			switch( QMessageBox::information(0, 
											"Disabling MetaGP",
											"You are about to disable the Meta GP-System. What shall\n"
											"we do with the system?",
											"Disable",
											"Remove",
											"Save & Remove",
											0, 0) )
			{
			case 2 :	// disables the system, saves it and finally delete it from memory
					// save the gpSystem
				saveName = QString();	// force the routine to show a filedialog
				if(!saveSystem(proposedName))
					return false;

			case 1 :	// disables the system and remove it from memory
						// remove it from memory
				if(mainWindow){
					QObject::disconnect(mainWindow->mtStartEvolutionAction, SIGNAL( triggered() ), this, SLOT( startSingleEvolution() ));
					QObject::disconnect(mainWindow->mtStopEvolutionAction, SIGNAL( triggered() ), this, SLOT( stopEvolution() ));
					QObject::disconnect(mainWindow->mtDefaultAction, SIGNAL( triggered() ), this, SLOT(slotLoadDefault() ) );
					QObject::disconnect(mainWindow->mtLoadAction, SIGNAL( triggered() ), this, SLOT( slotLoadSetup() ) );
					QObject::disconnect(mainWindow->mtSaveAction, SIGNAL( triggered() ), this, SLOT( slotSaveSetup() ));
					QObject::disconnect(mainWindow->evolTimer, SIGNAL( timeout() ), this, SLOT( stopEvolution() ));
				}

				delete gpManager;
				delete mainWindow;
				delete substitution;
				substCache.inUse = false;
				gpManager = 0;
				mainWindow = 0;
				substitution = 0;
				saveName = QString();
				confStrm.setDevice(nullptr);
				if(confFile.isOpen()) confFile.close();
			
			case 0 :	// disables the system and keeps it in memory
						// switch to normal fitness calculation / tournament
				metaOn = state;
				usedSystem = NOMETA_SUBST;
				break;
			}
		} else {
			// disables the system and keeps it in memory
			// switch to normal fitness calculation / tournament
			metaOn = state;
			usedSystem = NOMETA_SUBST;
		}
	}

	metaOn = state;
	usedSystem = selectedSystem;

	// update sig_mainwindow

	return true;
}

/***
 * create all needed parts of the gp-system
 ***/
bool MT_Controller::createGPSystem()
{
	// make sure we loaded a configuration
	if(!confStrm.device()){
		readFromFile(QString());	// load an experiment without a filename
										// this should result in loading the 
										// default configuration file
	}

	if(!confFile.isOpen()){
		confFile.open(QIODevice::ReadOnly);
	}

	// fill the substituter cache
	if(substCache.inUse == false)
		loadCache(confStrm);

	// make sure we have a gp-manager
	if(!gpManager){
		if(!(gpManager = new MT_GPManager(confStrm))){
			SIGEL_Tools::SIG_IO::cerr << "Creating Meta GP-System: couldn't create GP-Manager." << Qt::endl;
			return false;
		} else {
			if(int error = gpManager->getLastError() && guiEnabled){
				SIGEL_Tools::SIG_IO::cerr << "Creating GP-System: error in creating Meta-GP-System. Error no: "
						<< error << Qt::endl;
			}
		}
	}

	// everything went well (no error occurred or all errors could be recovered)
	return true;
}

/***
 * popup the configuration window
 * called by sigel mainwindow
 ***/
void MT_Controller::configureSystem()
{
	// if there is an up to date substituter, fill the substCache with 
	// the data from this substituter
	if(substitution){
		QBuffer buffer;					// create a buffered stream
		QTextStream inStrm(&buffer);

		buffer.open(QIODevice::WriteOnly);		// save the substituter to the stream
		substitution->writeToFile(inStrm);
		buffer.close();

		buffer.open(QIODevice::ReadOnly);		// load the cache from the stream
		loadCache(inStrm);
		buffer.close();
	}

	if(!gpManager || !substCache.inUse){
		if(!createGPSystem()){
			SIGEL_Tools::SIG_IO::cerr << "Configure meta system: couldn't create the GP system." << Qt::endl;
			return;
		}
	}
	if(!mainWindow){
		if(!(mainWindow = new MT_MainWindow(this, gpManager, &substCache, 0, "MTMainWindow"))){
			SIGEL_Tools::SIG_IO::cerr << "Configure meta system: couldn't open the configuration window." << Qt::endl;
			return;
		}
	}

	QObject::connect(mainWindow->mtStartEvolutionAction, SIGNAL( triggered() ), this, SLOT( startSingleEvolution() ));
	QObject::connect(mainWindow->mtStopEvolutionAction, SIGNAL( triggered() ), this, SLOT( stopEvolution() ));
	QObject::connect(mainWindow->mtDefaultAction, SIGNAL( triggered() ), this, SLOT(slotLoadDefault() ) );
	QObject::connect(mainWindow->mtLoadAction, SIGNAL( triggered() ), this, SLOT( slotLoadSetup() ) );
	QObject::connect(mainWindow->mtSaveAction, SIGNAL( triggered() ), this, SLOT( slotSaveSetup() ));
	QObject::connect(mainWindow->evolTimer, SIGNAL( timeout() ), this, SLOT( stopEvolution() ));

	mainWindow->show();
	delete substitution;
	substitution = 0;
}

/***
 * read the meta system if sigel reads the experiment
 * called from SIG_GPExperiment::loadExperiment(QTextStream & file)
 * if the loaded experiment uses meta
 ***/
bool MT_Controller::readFromFile(QString fileName)
{
	if(confFile.isOpen()){
		confFile.close();
	}
	if(confStrm.device()){
		confStrm.setDevice(nullptr);
	}

	bool stdConf = false;
	if(fileName.isNull()){
		stdConf = true;
		defConfFileName = ::getenv("SIGEL_ROOT");
		defConfFileName.append("/stdConf.mt");
		confFile.setFileName(defConfFileName);
	} else {
		confFile.setFileName(fileName);
	}

	if(confFile.open(QIODevice::ReadOnly)){

		if(!stdConf)
			saveName = fileName;		// remember where to save the system
		else
			saveName = QString();


	} else {
		if(guiEnabled){
			switch(QMessageBox::critical(0,
								"Loading experiment",
								"An error occurred in loading the meta experiment.\n"
								"Press <standard> to load the default setup or\n"
								"press <deactivate> to disable the meta system.",
								"Standard", "Deactivate", 0, 0))
			{
			case 0 :	// load default settings
				saveName = QString();
				confFile.setFileName(defConfFileName);
				if(!confFile.open(QIODevice::ReadOnly)){

					// oops, couldn't open default configuration file
					SIGEL_Tools::SIG_IO::cerr << "Loading experiment: couldn't load default settings. "
						"The meta system will be disabled." << Qt::endl;
				} else {
					break;
				}
			case 1 :	// disable meta system
				saveName = QString();
				useMeta(false);
				return false;
				break;
			}
		} else {
			pvm_halt();
			exit(1);
		}
	}

	// so far all went well, now attach the file to the stream and read
	// the data determined for the controller
	bool error = false;

	int idx = saveName.lastIndexOf(".mexp");
 	if(idx == -1){
		baseName = saveName;
	} else {
		baseName = saveName.left(idx);
		baseName.append(".mexp");
	}

	confStrm.setDevice(&confFile);
	QString tmpStr = confStrm.readLine();
	if(stdConf == true){
		usedSystem = selectedSystem;
		useMeta(true);
	} else if(int index = tmpStr.indexOf("usedSystem=") != -1 ){
		switch(tmpStr.mid(index+10).toInt()){
		case EVALUATOR_SUBST :
			usedSystem = EVALUATOR_SUBST;
			selectedSystem = usedSystem;
			useMeta(true);
			break;

		case CLASSIFIER_SUBST :
			usedSystem = CLASSIFIER_SUBST;
			selectedSystem = usedSystem;
			useMeta(true);
			break;

		case NOMETA_SUBST :
		default:
			error = true;
		}
		loadCache(confStrm);
	} else {
		// we are in trouble; loaded file has incorrect syntax
		// this should never happen;

		error = true;
	}

	if(error){
		if(guiEnabled)
			SIGEL_Tools::SIG_IO::cerr << "Loading experiment: error in loading experiment. "
						"The meta experiment file is corrupted. Disabling the meta system." << Qt::endl;
		useMeta(false);
		return false;
	}

	return true;
}

/***
 * save the meta system if sigel saves the experiment
 * called from SIG_GPExperiment::saveExperiment(QTextStream & file)
 ***/
void MT_Controller::writeToFile(QTextStream &file, QString name, int autoSave)
{
	if(metaOn){
		
		// wait for the evolution to end before saving the system
		// to prevent problems with the concurrent threads
#ifdef _WINDOWS
		if(gpManager)
			WaitForSingleObject(gpManager->evolutionMutex, INFINITE);
#else
		if(gpManager)
			pthread_mutex_lock(&gpManager->evolutionMutex);
#endif
			
		// save the meta system
		if(autoSave){
			if(!baseName.isEmpty())
				saveName = baseName + QString("%1").arg(autoSaveCnt = autoSaveCnt++ % 3);
			if(!saveSystem( name ))
				return;		// oops, something went wrong
		} else {
			if(!saveSystem(name))
				return;		// oops, something went wrong
		}

#ifdef _WINDOWS
		if(gpManager)
			ReleaseMutex(gpManager->evolutionMutex);
#else
		if(gpManager)
			pthread_mutex_unlock(&gpManager->evolutionMutex);
#endif

		
		file << "MetaInUse\n";					// write the file name for the meta experiment
		file << "fileName=" << saveName << "\n";// into the sigel experiment file
		file << "#####\n";
	}
}

/***
 * saves just the meta gp-system
 * leaves the used filename in saveName
 ***/
bool MT_Controller::saveSystem(QString sigExpName)
{
	// before we do anything lets close the configuration file we are reading from
	if(confFile.isOpen()){
		confFile.close();
	}

	bool stdConf = false;	// default configuration used ?
	QString proposedName("mt_");
	proposedName.append(sigExp.experimentName);		// proposed filename based on the name
													// of the corresponding sigel experiment
	// ask the user for a filename if no name is given so far
	if(saveName.isEmpty()){
		stdConf = true;
		if(guiEnabled)
			saveName = QFileDialog::getSaveFileName(nullptr, QString(), proposedName, "*.mexp;;*");
		else
			saveName = "lastExperiment.mexp";

		if(saveName.isEmpty())	// the user doesn't want to save the system anymore ? Bye!
			return false;
		if(saveName.right(5) != ".mexp")
			saveName.append(".mexp");

		baseName = saveName;
	}

	// open the file, create a filestream and save the system
	if(!gpManager && !stdConf)		// there's no gpManager therefore there was no change
		return true;				// --> no need to save anything

	QFile file(saveName);
	if(file.open(QIODevice::WriteOnly)){
		QTextStream fileStr(&file);

		fileStr << "usedSystem=" << usedSystem << "\n";		// remember the used type of system

		// save the substituter / the substituter settings
		if(substitution){
			substitution->writeToFile(fileStr);
		} else if(substCache.inUse){
			saveCache(fileStr);
		}

		// save the gp system
		if(gpManager){
			gpManager->writeToFileGPSystem(fileStr);			// save the rest
		}
		else
		{	
			if(stdConf){

				// system uses the default configuration
				// therefore copy the default configuration file
				QFile stdFile(defConfFileName);
				// Qt 2's QTextStream::read() did an unconditional s/\r\n/\n on the way in
				// (qtextstream.cpp:1531), with no dependence on the open mode. Qt 6's
				// readAll() returns raw bytes unless the device is opened with Text, so
				// without this a CRLF default-config file is copied verbatim into the
				// saved .mexp where 1.3 wrote LF.
				if(stdFile.open(QIODevice::ReadOnly | QIODevice::Text)){
					QTextStream stdStr(&stdFile);
					fileStr << stdStr.readAll();
					stdFile.close();
				} else
					return false;
			} // in the other case there was no change in the system 
			  // therefore there's no need to save it
		}

		file.close();		// finished - close everything
		file.open(QIODevice::ReadOnly);
	} else {
		// oops, something went wrong
		if(guiEnabled)
			SIGEL_Tools::SIG_IO::cerr << "Saving experiment: couldn't save the experiment." << Qt::endl;
		return false;
	}

	return true;
}


int MT_Controller::UsedSystem()
{
	return usedSystem;
}


bool MT_Controller::IsEnabled()
{
	return metaOn;
}


MT_Substitute* MT_Controller::getFitnessTrainer()
{
	if(UsedSystem() == EVALUATOR_SUBST){
		createGPSystem();
		if(!substitution){
			createCacheStrm();
			substitution = new MT_Evaluator(sigExp, *cacheStrm);
		}
		return substitution;
	}
	else
		return 0;
}

MT_Substitute* MT_Controller::getClassifier()
{
	if(UsedSystem() == CLASSIFIER_SUBST){
		createGPSystem();
		if(!substitution){
			createCacheStrm();
			substitution = new MT_Classifier(*cacheStrm);
		}
		return substitution;
	}
	else
		return 0;
}

/***
 * all the functions for the substituter cache
 **/
void MT_Controller::loadCache(QTextStream &File)
{
	QString Classifier( "Classifier:" );
	QString Evaluator( "Evaluator:" );

	QString PresentLine = File.readLine();

	while((PresentLine != Classifier) && (PresentLine != Evaluator) && !(File.atEnd()))
		PresentLine = File.readLine();

	int savedSystem = 0;
	if(PresentLine == Classifier){
		savedSystem = CLASSIFIER_SUBST;
	} else if(PresentLine == Evaluator){
		savedSystem = EVALUATOR_SUBST;
	}

	substCache.numCorrectEst = 0;
	substCache.numMetaEst = 0;
	if(usedSystem == savedSystem){
		substCache.strategy = (File.readLine()).toInt();
		substCache.tolerance = (File.readLine()).toDouble();
		substCache.refreshInt = (File.readLine()).toInt();
		substCache.genNumber = (File.readLine()).toUInt();
		if(substCache.genNumber != 0){
			correctEst.resize(substCache.genNumber);
			metaEst.resize(substCache.genNumber);
						
			for(int i=0; i< substCache.genNumber; i++)
			{
				correctEst[i] = (File.readLine()).toUInt();
				metaEst[i] = (File.readLine()).toUInt();
			}
			substCache.numCorrectEst = &correctEst;
			substCache.numMetaEst = &metaEst;
		}

	} else {		// default setup

		QFile defFile(defConfFileName);
		QTextStream defStrm(&defFile);
		defFile.open(QIODevice::ReadOnly);

		switch(usedSystem){
		case CLASSIFIER_SUBST :

			// load a classifier
			while ((PresentLine != Classifier) && !(defStrm.atEnd()))
				PresentLine = defStrm.readLine();
			if ((PresentLine == Classifier) && !(defStrm.atEnd())){
				substCache.strategy = (defStrm.readLine()).toInt();
				substCache.tolerance = (defStrm.readLine()).toDouble();
				substCache.refreshInt = (defStrm.readLine()).toInt();
				substCache.genNumber = (defStrm.readLine()).toUInt();
			}
			break;

		case EVALUATOR_SUBST :
	
			// load an evaluator
			while ((PresentLine != Evaluator) && !(defStrm.atEnd()))
				PresentLine = defStrm.readLine();
			if ((PresentLine == Evaluator) && !(defStrm.atEnd())){
				substCache.strategy = (defStrm.readLine()).toInt();
				substCache.tolerance = (defStrm.readLine()).toDouble();
				substCache.refreshInt = (defStrm.readLine()).toInt();
				substCache.genNumber = (defStrm.readLine()).toUInt();
			}
			break;
		}
	}
	substCache.inUse = true;
}

void MT_Controller::saveCache(QTextStream &File)
{
	switch(usedSystem){
	case CLASSIFIER_SUBST :
		File << ("Classifier:\n");
		break;

	case EVALUATOR_SUBST :
		File << ("Evaluator:\n");
		break;
	}
	File << substCache.strategy <<Qt::endl;
	File << substCache.tolerance <<Qt::endl;
	File << substCache.refreshInt <<Qt::endl;
	File << substCache.genNumber <<Qt::endl;
	if(substCache.genNumber != 0){
		for(int i=0; i< substCache.genNumber; i++){
			File << correctEst[i] << "\n";
			File << metaEst[i] << "\n";
		}
	}
}

void MT_Controller::createCacheStrm()
{
	if(usedSystem == EVALUATOR_SUBST){
		cacheString = "Evaluator:\n";
	} else {
		cacheString = "Classifier:\n";
	}
	cacheString.append(QString("%1\n").arg(substCache.strategy));
	cacheString.append(QString("%1\n").arg(substCache.tolerance));
	cacheString.append(QString("%1\n").arg(substCache.refreshInt));
	cacheString.append(QString("%1\n").arg(substCache.genNumber));
	if(substCache.genNumber != 0){
		for(int i=0; i<substCache.genNumber; i++){
			cacheString.append(QString("%1\n%2\n").arg(correctEst[i]).arg(metaEst[i]));
		}
	}
	
	delete cacheStrm;
	cacheStrm = new QTextStream(&cacheString, QIODevice::ReadOnly);
}

/***
 * implementation of main toolbar actions
 ***/
void MT_Controller::slotLoadDefault()
{
	QFile file(defConfFileName);
	if(file.open(QIODevice::ReadOnly)){

		QTextStream strm(&file);
		delete substitution;
		substitution = 0;

		// get to know what kind of system to use
		QString tmpStr = strm.readLine();
		if(int index = tmpStr.indexOf("usedSystem=") != -1 ){
			switch(tmpStr.mid(index+10).toInt()){
			case EVALUATOR_SUBST :
				usedSystem = EVALUATOR_SUBST;
				useMeta(true);
				break;

			case CLASSIFIER_SUBST :
				usedSystem = CLASSIFIER_SUBST;
				useMeta(true);
				break;

			case NOMETA_SUBST :
			default:
				break;
			}
		}

		// load substituter and gp system setup
		loadCache(strm);
		gpManager->loadSetup(strm);

		mainWindow->enforceUpdate(true);
		file.close();

	} else {		// read error
		SIGEL_Tools::SIG_IO::cerr << "Load setup: couldn't load default setup. Aborting operation." << Qt::endl;
	}
}

void MT_Controller::slotLoadSetup()
{
	QString fileName = QFileDialog::getOpenFileName(mainWindow, QString(), QString(), "Setup(*.mcnf);;All Files(*)");
	
	if(fileName.isEmpty())
		return;

	QFile file(fileName);
	if(file.open(QIODevice::ReadOnly)){

		QTextStream strm(&file);
		delete substitution;
		substitution = 0;

		// get to know what kind of system to use
		QString tmpStr = strm.readLine();
		if(int index = tmpStr.indexOf("usedSystem=") != -1 ){
			switch(tmpStr.mid(index+10).toInt()){
			case EVALUATOR_SUBST :
				usedSystem = EVALUATOR_SUBST;
				useMeta(true);
				break;

			case CLASSIFIER_SUBST :
				usedSystem = CLASSIFIER_SUBST;
				useMeta(true);
				break;

			case NOMETA_SUBST :
			default:
				break;
			}
		}

		// load substituter and gp system setup
		loadCache(strm);
		gpManager->loadSetup(strm);

		mainWindow->enforceUpdate(true);
		file.close();

	} else {		// read error
		SIGEL_Tools::SIG_IO::cerr << "Load setup: couldn't open specified file. Aborting operation." << Qt::endl;
	}
}

void MT_Controller::slotSaveSetup()
{
	QString fileName = QFileDialog::getSaveFileName(mainWindow, QString(), QString(), "Setup(*.mcnf);;All Files(*)");

	if(fileName.isEmpty())
		return;

	if(fileName.right(5) != ".mcnf")
		fileName.append(".mcnf");

	QFile file(fileName);
	if(file.exists()){
		if(0 == QMessageBox::warning(0, "Save configuration", "There is another file with this name. This will overwrite\n"
			"the existing file. Do really want to continue?", "Ok", "Cancel", 0, 1))
			return;
	}
	if(file.open(QIODevice::WriteOnly)){

		QTextStream strm(&file);

		strm << "usedSystem=" << usedSystem << "\n";		// remember the used type of system

		mainWindow->enforceUpdate(false);		// enforce an update of the gp-system

		// save the substituter / the substituter settings
		/*if(substitution){
			substitution->writeToFile(strm);
		} else*/
		if(substCache.inUse){
			saveCache(strm);
		}

		// save the gp system
		if(gpManager){
			gpManager->writeToFileSetup(strm);			// save the rest
		}
		file.close();

		mainWindow->enforceUpdate(true);		// enforce an update of the GUI

	} else {
		SIGEL_Tools::SIG_IO::cerr << "Load setup: couldn't open specified file. Aborting operation." << Qt::endl;
	}
}