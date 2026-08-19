// MT_Randomizer.h: interface for class MT_Randomizer.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_RANDOMIZER_H__65257FEF_679C_4B32_853B_E2AE4688CAD6__INCLUDED_)
#define AFX_MT_RANDOMIZER_H__65257FEF_679C_4B32_853B_E2AE4688CAD6__INCLUDED_

#include "MT_GPSystem/MT_Operand.h"	
#include "MT_GPSystem/MT_Instruction.h"

#include <qstring.h>
#include <qtextstream.h>
#include <qqueue.h> 
#include <qarray.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// #define RAND_MAX 65534

/* the Randomizer supply "correct" Coincidence to the whole GP-System 
* it's contain a lot of GP parameters 
*/
class MT_Randomizer  
{
public:

	/******* administrative methods *******/
	MT_Randomizer (QTextStream &File);
	MT_Randomizer();
	virtual ~MT_Randomizer();
	void writeToFileRandi (QTextStream &File);
	void loadSetup (QTextStream &File);
	
	/******* special method for supply random *******/
	
	/*supply a random search operator
	* 1 indicate search per recombination (Crossover)
	* 2 indicate search per mutation 
	* 3 indicate search per reproduction (Copy)
	*/
	int getRandomSOperator();
	
	/* supply a ppinter of the XPointarray, which contain the posistion for a crossover event
	*/
	QArray<int>* getRandomXPoints (int SizeIndi1, int SizeIndi2);
	
	/* serve the random mutate;
	* supply an answer, if a program line or element of a line should mutate */
	bool answerMutateLine ();
	bool answerMutateElement();
	
	/* method to yield random instruction, variable or a Operand
	* for creating a randm program line or mutate a line  */
	MT_Instruction getRandomInstruktion();
	MT_Operand * getRandomVariable();
	MT_Operand * getRandomOperand ();

	/*  supply the program Length for generating individuals*/	
	int getProgLength();

	/*serve the random creat of NumOfConstant Constnant
	* of the type int if Typ true else double between underBoun and upperboun */
	void createConstant (int NumOfConstant, bool Type , double underBoun, double upperBoun);
	
	/* method for fitness proportional selection,
	* supply the position of the winner*/
	int getProportionalWinner (QArray<int> * Players);
	
	/* supply a partition of the offspring into tournaments
	* a entry (0-(num-1)) indicate in which tournament separate a individual form the offspring
	*/
	QArray<int> * getRandomTournamentPartition(int NumberOfTour);


	/******* normal set/ get method *******/
	
	/* method for changing information between GUI and Randomizer; this two method bend the given pointer
	* to the private attribute; so the GUI can change (or get) direct information from the Randomizer
	*/
	void returnSearchValue(QArray<double>  **ProbMPower, QArray<double>  **ProbSOperator, QArray<double> **ProbXPoints);
	void returnIndividualsValue(int **Length, int **NumOfVar, QArray<double> **Con, QArray<double> **ProbOfFu);

	/* change the Offspring Size*/
	void setOffspringSize (int setOffSize);
	
	/* ATTENTION: set ParentSize change TournamentNumber, TournamantSize and ParentSize !*/
	void setParentSize (int SizeOfParent);

	int getNumOfVari();
	int getProgramLengthMax();
	int getLastError();

	/* supply a random unsigned long in [0, upperBoundary)*/
	unsigned long getRandomInteger(unsigned long upperBoundary);

private:

	int LastError;

	/* contain the position for a crossover Event
	* Array[0] = indicate the number of X points for this crossover event
	* Array[1] = the firste XPoint in the first Individual
	* Array[2] = the firste XPoint in the second Individual
	* Array[3] = the second XPoint in the first Individual
	* Array[4] = the second XPoint in the second Individual
	* Array[5] = the third XPoint in the first Individual
	* Array[6] = the third XPoint in the second Individual
	*/
	QArray<int> RandomXPoints;

	
	/* this array serve for indicate the tournament member of a tournament */
	QArray <int> TournamentPartition;
	
	/* a lot of size of variously GP object */
	int NumberOfVariables;
	int ProgramLengthMax;
	int OffspringSize;
	int ParentSize;

	//////////////////////////////////////////////////////////////////////////////////////////////////
	// a lot of array, which contain the relative probability and not the real probability
	// that mean e.g. the  probability of the Search Operators are 30,5% (Crossover) 22,25% (Mutation)
	// and 52,75% Reproduction so are the entries in the array: 3050 / 2225/ 5275.
	//////////////////////////////////////////////////////////////////////////////////////////////////

	/* contain the probability of the SearchOperators, which are desired from the user
	* ProbSearchOperator[0]= probability of Crossover
	* ProbSearchOperator[1]= probability of Mutation
	* ProbSearchOperator[2]= probability of Reproduction
	*/
	QArray <double> ProbSearchOperator;
		
	/* contain the probability of occurrence of a mutation which are desired from the user,
	* ProbMutationPower[0] = the probability, that a program line will choose for mutation.
	* ProbMutationPower[1] = the probability, that a element of a program line will choose for mutation
	* @pre: 0<= Entry <=1000
	*/
	QArray <double> ProbMutationPower;

	/* contain the probability for chosen the Number of X-Points during a crossover event
	*ProbCrossOverPoints[0]= the probaility of 1 Crossover Point
	*ProbCrossOverPoints[1]= the probaility of 2 Crossover Point
	*ProbCrossOverPoints[2]= the probaility of 3 Crossover Point
	*/
	QArray <double> ProbCrossOverPoints;
	
	/* contain all constant, which the GP-System allow to use. */
	QArray <double> Constant;

	/* contain the probability for chosen a Instruction 
	* by the mutation or the generation of a program line
	* ProbInstruktion[0] = probability of SWT 
	* ProbInstruktion[1] = probability of CPY  ;ProbInstruktion[2]= probability of ADD
	* ProbInstruktion[3] = probability of SUB  ;ProbInstruktion[4]= probability of MUL
	* ProbInstruktion[5] = probability of DIV  ;ProbInstruktion[6]= probability of MOD
	* ProbInstruktion[7] = probability of MIN  ;ProbInstruktion[8]= probability of MAX
	*
	* ProbInstruktion[9] = probability of CMP  ;ProbInstruktion[10]= probability of JMP
	* ProbInstruktion[11] = probability of JLE  ;ProbInstruktion[12]= probability of JEQ
	* ProbInstruktion[13]= probability of NOP
	* ProbInstruktion[14] = probability of LUI ;ProbInstruktion[15]= probability of LUS
 	* ProbInstruktion[16] = probability of LUT ; ProbInstruktion[17] = probability of LUM ;
	*/
	QArray <double> ProbInstruktion;


};

#endif // !defined(AFX_MT_RANDOMIZER_H__65257FEF_679C_4B32_853B_E2AE4688CAD6__INCLUDED_)
