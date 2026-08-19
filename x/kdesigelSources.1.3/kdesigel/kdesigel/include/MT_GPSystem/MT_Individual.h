// MT_Individual.h: interface for class MT_Individual.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_INDIVIDUAL_H__630909EE_4F20_4811_94CA_2F9AB46D6046__INCLUDED_)
#define AFX_MT_INDIVIDUAL_H__630909EE_4F20_4811_94CA_2F9AB46D6046__INCLUDED_

#include <qstring.h>
#include <qtextstream.h>
#include <qarray.h> 
#include "MT_GPSystem/MT_Program.h"
#include "MT_GPSystem/MT_Randomizer.h"	


/*This class represent a Meta Individual, which contain especially the program 
* for Interpreting  
*/

class MT_Individual  

{

public:
	
	QString printProgramLine(int index);
	

	/* change the max Numbers of Variable, which can use by the Interpreter
	* if a variable name > NewNum so it will set of (variable name) mod NewNum
	*@post: all variable names are between 0 and NewNum-1 of all program lines */
	void changeMaxNumVariable(int NewNum);

	/* this function determine whether the currently fitness estimate for this Individuals, 
	* if this done then the function returns true */
	bool toBeEvaluated(int TSet);

	// increase the Age; Age = Age+1; 
	void increaseAge();

	/* a lot of get and set function for handling the attributes */
	void setTrainingsSet (int TName);
	void setFitness(double NewFitness);
	void setPosition(int Pos);
	void setMaxProgLen(int NewLen);
	void setTypOfGenesis(int Typ);
	void setFitnessOfParent (double FitOfParent);
	void setNewProgram(MT_Program * NewProg);
	void setNewAge(int NewAge);
	void setNewName(int NewName);


	int getTypOfGenesis();
	double getFitnessOfParent ();
	int getPosition();
	double getFitness();
	int getName();
	int getAge();
	int getLastError();
	MT_Programline * getProgramLine(int Line);
	MT_Program * getProgram();

	// create a new Program, the older program will delete	
	void createProgram(MT_Randomizer *Randi);

	MT_Individual(MT_Program *Prog, bool Copy);
	
	MT_Individual(QTextStream &File);
	MT_Individual(int Na, MT_Randomizer *Randi);
	void writeToFileIndi(QTextStream &File);
	MT_Individual();
	virtual ~MT_Individual();

private:

	int LastError;
	/* indicate the Fitness of a Parent; -1 = Parent not existing */
	double FitnessOfParent;

	/* indicate how the program were created
	* -1= not defined  ; 0= Random; 1= OneCrossoverPoint; 2=OneCrossoverPointMutation;
	* 3 = OneCrossoverPoint; 4= Reproduktion; 5=Elter; 100+X = X Mutatuion have performed
	* 6 = a loaded individual
	*/
	int TspOfGenesis;

	// the name of the Trainingset on which the Individual training last time 
	int TSetName;
	// indicate the location of the individual inside the population 
	int Position;
	// indicate the Age of the individual
	int Age;
	int Name;
	//the main part of the META individual, a pointer of the program 
	MT_Program * Program;
	// the fitness of the Progrom, semantic: greater fitness are better;     
	double Fitness;
};

#endif // !defined(AFX_MT_INDIVIDUAL_H__630909EE_4F20_4811_94CA_2F9AB46D6046__INCLUDED_)
