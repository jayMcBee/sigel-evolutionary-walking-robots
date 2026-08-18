// MT_Substitute.h: Schnittstelle für die Klasse MT_Substitute.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_SUBSTITUTE_H__1103ED83_4D0E_4EC2_BFE5_3D4A0E49FCD6__INCLUDED_)
#define AFX_MT_SUBSTITUTE_H__1103ED83_4D0E_4EC2_BFE5_3D4A0E49FCD6__INCLUDED_



#include "MT_GPSystem/MT_Program.h"	// Hinzugefügt von der Klassenansicht
#include "MT_GPSystem/MT_Interpreter.h"	// Hinzugefügt von der Klassenansicht
#include "MT_GPSystem/MT_TranslatedIndividual.h"
#include "MT_GPSystem/MT_TrainingCase.h"
#include "MT_GPSystem/MT_TrainingCase.h"

#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_Program/SIG_ProgramLine.h"

#ifdef _WINDOWS
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <qstring.h>
#include <qtextstream.h>
#include <qqueue.h> 
#include <qarray.h>
#include <math.h>

struct subst_cache {
	bool inUse;
	int strategy;
	int refreshInt;
	double tolerance;
	unsigned int genNumber;
	QArray<unsigned int> *numCorrectEst;
	QArray<unsigned int> *numMetaEst;
};

class MT_Substitute  
{

public:
	
	// get/set method for Parameter update
	void getEstimationParameter(int *EStrategy, double *Tol, int *ReInterval);
	void setEstimationParameter(int EStrategy, double Tol, int ReInterval);
	void setInterpreter(int NumOfVariable,  int TimeToInter);
	void getNumOfEstimation(QArray<unsigned int> *  MetaEstimation, QArray<unsigned int>  *  CorrectEstimation);
	int getTyp();

	/*nextSIGGeneration() indicate the evaluator, that a new SIGEL Generation started */
	void nextSIGGeneration(double AverageSigelFit);

#ifdef _WINDOWS
	HANDLE interpreterMutex;
	HANDLE tCaseBufferMutex;
	HANDLE fitnessMutex;
#else
	pthread_mutex_t interpreterMutex;
	pthread_mutex_t tCaseBufferMutex;
	pthread_mutex_t fitnessMutex;
#endif

	// changeTCases() supply the new TrainingCases for the Meta GP-System
	QQueue<MT_TrainingCase> * changeTCases();

	// update the BestMETAProgram;  
	void changeBest(MT_Program * MetaProg);

	// used for update the MetaError 
	void changeErrorInfo(QArray<double> * OutCome, QArray<double> * CorrectFit);

	//überladende Methoden 
	MT_Substitute();
	virtual ~MT_Substitute();
	virtual void loadSetup(QTextStream &File);
    virtual void writeToFile(QTextStream &File);
	virtual void writeToFileSetup(QTextStream &File);

private:


protected:
	
	// Sigel Generation Number
	int GenerationNumber;
	
	// translated a SIGEL Program to three arrays; which by a MetaProgram can used  
	MT_TranslatedIndividual * translatedSIGProg(SIGEL_Program::SIG_Program *SIGProg);

	// the Program of the present best Meta Individuals 
	MT_Program * BestMETAProgram;
	
	// for EstimationStrategy: count the Meta Estimation between the calibrationsteps   
	int NumOfMeta;
	
	//for EstimationStrategy: indicate the goodness of the BestMETAProgram
	double MetaProgError;
	
	// for EstimationStrategy: to determinate the MetaProgError, the Trainingsdata are needed  
	QArray<double> CorrectFitness;
	QArray<double> AssumedFitness; 

	// the AverageFitness of the Sigel GP Population for the last Generation
	// supply for the evaluationTactic
	double AverageSigelFitness;
	
	// buffer the New MT_TrainingCase;
	QQueue<MT_TrainingCase> TCaseBuffer;

	// to interpret a SigelProgram on the BestMETAProgram
	MT_Interpreter * Interpreter;

	/*inidcate the Typ of Substitute
	* 1:= Evaluator 2:= classifier  
	*/ 
	int Typ;

	/* Gitb die Stratgie vor, wie die Frage zu beantwortet ist, ob Meta oder Simulation zur 
	* Bestimmung der Sigel-Fitness benutzt werden soll.
	* Beim Evaluator:
	* EstimationStrategy = 1 := durchschnitliche Abweichung von der Fitness < Tolerance; Tolerance>=0;
	* EstimationStrategy = 2 := durchschnitliche Abweichung von der Fitness ohne(***)  < Tolerance;
	* (***) := die (Anzahl der Fitnessfälle)/10 größten Abweichnungen 
	* EstimationStrategy = 3 := percentaged Anzahl von falsche Endscheidungen: 0% < Tolerance < 100%
	* EstimationStrategy = 4 :=
	* EstimationStrategy sonst := 
	*/
	int EstimationStrategy;
	// for EstimationStrategy: 
	double Tolerance;
	// for EstimationStrategy: 
	int RefreshInterval;

	// different variable for count; 
	QArray<unsigned int> NumOfCorrectEstimation;
	QArray<unsigned int> NumOfMetaEstimation;

};

#endif // !defined(AFX_MT_SUBSTITUTE_H__1103ED83_4D0E_4EC2_BFE5_3D4A0E49FCD6__INCLUDED_)
