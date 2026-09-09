// MT_TrainingCase.h: interface for class MT_TrainingCase.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_TRAININGCASE_H__A1E78B17_E63C_4CF7_9AC7_A9BE752FF329__INCLUDED_)
#define AFX_MT_TRAININGCASE_H__A1E78B17_E63C_4CF7_9AC7_A9BE752FF329__INCLUDED_

#include "MT_GPSystem/MT_TranslatedIndividual.h"	// Added from the class view
#include <qstring.h>
#include <qtextstream.h>

/* This class contain a trainingcase for the GP-System;
* It consists of a translated SIGEL individual (i.e. the robot control program)
* and additional Information e.g. the precisely fitness of the SIGEL Individual.
*/

class MT_TrainingCase  
{

private:		
	/* The translated SIGEL Individual (respectively the robot control program)
	*/
	MT_TranslatedIndividual * TranslateIndividual;
	
	/* The boundary indicate the end of the first Individual.  
	*/
	int Boundary;
	
	/* The name serve only for identification.
	* it will be setting on the TaskID of the PVM-task (see also MT_Evaluator::spawnTask)
	*/
	int Name;

	/* The precisely fitness of the SIGEL Individual,
	* which were determine by simulation of the robot control program.
	*/
	double Fitness;

public:	
	void setFitness(double Fit);
	int getName();
	MT_TrainingCase(QTextStream &File);
	/* The constructor initializing TrainingCase with the data from a Qstring, by loading an experiment.
	* @pre: There is TrainingCase data, encoded in a QString.
	* @post: a new TrainingCase is created from the data.
	*/
	
	/* the constructor receive the exact fitness, the translated Individual and the name. 
	*/
	MT_TrainingCase(double Fit, MT_TranslatedIndividual *TransIndi, int Na, int boun);
	
	MT_TrainingCase();
	virtual ~MT_TrainingCase();
	
	/* change/set  the Name of the T-case
	*/
	void setName(int NewName);
	
	/* change/set the Boundary
	*/
	void setBoundary(int NewBoundary);
	
	/* supply the precisely fitness of the SIGEL Individual
	*/
	double getFitness();
	
	/* supply the translated SIGEL Individual
	*/
	MT_TranslatedIndividual * getIndividual();

	/* This function writes a T-case to a QTextStream.
	* @param:
	* @post: The individual is written to the given QTextStream.
	*/
	void writeToFileTCase(QTextStream &File);

};

#endif // !defined(AFX_MT_TRAININGCASE_H__A1E78B17_E63C_4CF7_9AC7_A9BE752FF329__INCLUDED_)
