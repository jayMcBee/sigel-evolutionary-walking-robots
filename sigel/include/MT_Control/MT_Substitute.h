#ifndef MT_CONTROL_MT_SUBSTITUTE_H
#define MT_CONTROL_MT_SUBSTITUTE_H

#include <QQueue>
#include <memory>
#include "MT_Control/MT_EstimationStrategy.h"
#include "MT_Control/MT_SubstituteKind.h"
#include "MT_GPSystem/MT_Program.h"
#include "MT_GPSystem/MT_Interpreter.h"
#include "MT_GPSystem/MT_TranslatedIndividual.h"
#include "MT_GPSystem/MT_TrainingCase.h"

#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_Program/SIG_ProgramLine.h"

#include <pthread.h>

#include <qstring.h>
#include <qtextstream.h>
#include <math.h>

class MT_Substitute
{
public:
	// get/set method for Parameter update
	void getEstimationParameter( MT_EstimationStrategy *EStrategy, double *Tol, int *ReInterval );
	void setEstimationParameter( MT_EstimationStrategy EStrategy, double Tol, int ReInterval );

	// The kind of substitute that uses the strategy.
	static MT_SubstituteKind kindOfStrategy( MT_EstimationStrategy strategy );
	void setInterpreter( int NumOfVariable, int TimeToInter );
	void getNumOfEstimation( QList<unsigned int> *MetaEstimation, QList<unsigned int> *CorrectEstimation );

	/*nextSIGGeneration() indicate the evaluator, that a new SIGEL Generation started */
	void nextSIGGeneration( double AverageSigelFit );

	pthread_mutex_t interpreterMutex;
	pthread_mutex_t tCaseBufferMutex;
	pthread_mutex_t fitnessMutex;

	// changeTCases() supply the new TrainingCases for the Meta GP-System
	QQueue<MT_TrainingCase *> *changeTCases();

	// update the BestMETAProgram;
	void changeBest( MT_Program *MetaProg );

	// used for update the MetaError
	void changeErrorInfo( QList<double> *OutCome, QList<double> *CorrectFit );

	//overriding methods
	MT_Substitute();
	virtual ~MT_Substitute();
	virtual void loadSetup( QTextStream &File );
	virtual void writeToFile( QTextStream &File );
	virtual void writeToFileSetup( QTextStream &File );

private:
	static constexpr int startNumberOfVariables = 10;
	static constexpr int startDuration = 100;

protected:
	// Sigel Generation Number
	int GenerationNumber;

	// translated a SIGEL Program to three arrays; which by a MetaProgram can used
	MT_TranslatedIndividual *translatedSIGProg( SIGEL_Program::SIG_Program const *SIGProg );

	// the Program of the present best Meta Individuals
	std::unique_ptr< MT_Program > BestMETAProgram;

	// for EstimationStrategy: count the Meta Estimation between the calibrationsteps
	int NumOfMeta;

	//for EstimationStrategy: indicate the goodness of the BestMETAProgram
	double MetaProgError;

	// for EstimationStrategy: to determinate the MetaProgError, the training data are needed
	QList<double> CorrectFitness;
	QList<double> AssumedFitness;

	// the AverageFitness of the Sigel GP Population for the last Generation
	// supply for the evaluationTactic
	double AverageSigelFitness;

	// buffer the New MT_TrainingCase;
	QQueue<MT_TrainingCase *> TCaseBuffer;

	// to interpret a SigelProgram on the BestMETAProgram
	MT_Interpreter Interpreter;

	// Decides when MetaGP estimates instead of SIGEL; it compares an error against Tolerance.
	MT_EstimationStrategy EstimationStrategy;

	// for EstimationStrategy:
	double Tolerance;

	// for EstimationStrategy:
	int RefreshInterval;

	// different variable for count;
	QList<unsigned int> NumOfCorrectEstimation;
	QList<unsigned int> NumOfMetaEstimation;
};

#endif // MT_CONTROL_MT_SUBSTITUTE_H
