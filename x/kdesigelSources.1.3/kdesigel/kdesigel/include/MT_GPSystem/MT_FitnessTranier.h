// MT_FitnessTranier.h: Schnittstelle für die Klasse MT_FitnessTranier.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_FITNESSTRANIER_H__8C160767_FE7C_4FFF_B2EC_DFA03F1E983A__INCLUDED_)
#define AFX_MT_FITNESSTRANIER_H__8C160767_FE7C_4FFF_B2EC_DFA03F1E983A__INCLUDED_

#include "MT_GPSystem/MT_TranslatedIndividual.h"
#include "MT_GPSystem/MT_TrainingCase.h"
#include "MT_GPSystem/MT_Interpreter.h"	
#include "MT_GPSystem/MT_Trainingset.h"	
#include "MT_GPSystem/MT_Program.h"
#include "MT_GPSystem/MT_Individual.h"
#include "MT_GPSystem/MT_Population.h"

#include <qstring.h>
#include <qtextstream.h>
#include <qqueue.h> 
#include <qarray.h>
#include <math.h>


/*This  class manages the fitness calculation of the MT_Individuals. 
*/
class MT_FitnessTranier  
{

private:

	int LastError;
	
	/*Indicate which FitnessFunktion is choose for the fitness calculation
	@param: 1 = fitSimpleError for the evaluator  approach 
	@param: 2 = fitSquareError for the evaluator  approach 
	@param: 3 = simpleYesNo  for the classifer approach 
	@param: 4 = weightYesNo  for the classifer approach 
	*/
	int FitnessFunction;
	
	/* for the evaluator  approach:
	* the both Result arrays serve for storing the fitness value
	* of the single fitness cases (exact and by the MT_Program)
	* for the classifer approach:
	* the both Resultarray contain: (Fitness of the first SigelProgram) -(Fitness of the second SigelProgram)
	* so if the value negative the second SigelProgram is the tournament winner, else the fist Program ...   
	*/
	QArray <double> Result;
	QArray <double> ResultIst;

	/* the Tset contain the T-cases*/
	MT_Trainingset * TSet;
	
	/* the PresentTSize represent the Number of T-cases in the T-Set  */
	int PresentTSize;

	/* the TSize represent the Number of T-case, which can contained max */
	int TSetSize;

	/* the Interpret peform the MT_Program 
	* on the SIGLE translated Individual */
	MT_Interpreter Interpreter;


public:

	int getLastError();

	/* this methode are needed for MT_GPManager::exchangeBest()
	* the Substituter needed the both ResultArrys 
	*/
	int getPresentTSize();
	QArray<double> * getResultArray();
	QArray<double> * getResultIstArray();

	int getTDuration();

	/* The constructor initializing MT_FitnessTrainer with the data from a Qstring,
	* by loading an experiment.
	* @pre: There is FitnessTrainer data, encoded in a QString.
	* @post: a new Trainer is created from the data.
	*/

	MT_FitnessTranier();
	virtual ~MT_FitnessTranier();
	MT_FitnessTranier (QTextStream &File);

	/* calculate the fitness of a given MT_Population
	* set the new calculate fitness directly in the Individual
	* @post: 0<Fitness<10 000 000 
	*/
	void calculateFitness (MT_Population * Pop);
	
	/* calculate the fitness of a given MT_Indivdual on T-Set 
	* set the new calculate fitness directly in the Individual.
	* @post: 0<Fitness<10 000 000 
	*/
	bool calculateFitness (MT_Individual * Indi);

	/* update the TSet,
	* receive a pointer of a QQueue, which contain new T-cases. 
	* The new T-cases result from the MT_Substitute.*/
	int insertNewTCases(QQueue<MT_TrainingCase> *NewTCase);
	
	/* change the Number of Variables, which the Interpreter allowed to use. */
	void setNumberOfVariables(int varNumber);
	
	/* change the SelektionValue because the GUI have change.
	* post: the FitnessFunction, the TrainingDuration and the TrainingSetSize are now up to date
	*/
	void setSelektionValue(int FitFunction, int TDuration, int TSize);
	
	/* the GUI isn't up to date (e.g. load setup), this function bring the GUI up to date.
	* the Value/ Data are directly change in the GUI by the given pointers.
	* @post: the FitnessFunction, the TrainingDuration and the TrainingSetSize are now up to date
	*/ 
	void getSelektionValue(int *FitFunction, int * TDuration, int *TSize);
	
	/* This function writes the MT_FitnessTrainer to a QTextStream.
	* @param:
	* @post: The T-Set is written to the given QTextStream.
	*/
	void writeToFileTrainer(QTextStream &File);
	
	/* This function load a Setup by the given QTextStream.
	* @pre: There is FitnessTrainer data, encoded in a QTextStream.
	* @post: the FitnessTrainer have a new Setup now.
	*/
	void loadSetup (QTextStream &File);
	
	void writeToFileSetup(QTextStream &File);

private:
	// fitnessfunction for the classifer approach 
	// @pre: PresentTSize>0; else div by Null!
	double weightYesNo();
	double simpleYesNo();
	
	// fitnessfunction for the evaluator  approach  
	double fitSimpleError();
	double fitSquareError();

};

#endif // !defined(AFX_MT_FITNESSTRANIER_H__8C160767_FE7C_4FFF_B2EC_DFA03F1E983A__INCLUDED_)
