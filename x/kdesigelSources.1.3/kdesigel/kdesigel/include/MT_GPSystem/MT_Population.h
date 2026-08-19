// MT_Population.h: interface for class MT_Population.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_POPULATION_H__76BD10AD_932E_42FC_BDD3_85DD57B1D541__INCLUDED_)
#define AFX_MT_POPULATION_H__76BD10AD_932E_42FC_BDD3_85DD57B1D541__INCLUDED_

#include "compat/q2compat.h"
#include "MT_GPSystem/MT_Program.h"
#include "MT_GPSystem/MT_Individual.h"
#include "MT_GPSystem/MT_Randomizer.h"	

#include <qstring.h>
#include <qtextstream.h>


/* this class administer the Individuals from the META - GP-System
*/
class MT_Population  
{
public:

	/* supply a pointer on the best individual*/
	MT_Individual * getBestIndividual();

	int getLastError();
	
	/* change the present PopSize to NewPopSize
	* this is a special method for the offspring population
	* ATTENTION:
	* @pre: the population must be empty.
	* @post: the population were resised; 
	*/	
	bool changePopSize(int NewPopSize);

	void setFreePos(int Pos);
	int getFreePos();
	
	/* change the max Numbers of Variable, which can use by the Interpreter
	* if a variable name of a MT_Program > NewNum so it will set of (variable name) mod NewNum
	*@post: all individuals have correct variable names !*/
	void changeMaxNumVariable(int NewNum);

	/* Change the maximum length an individual may reach; individuals may need shortening  */
	void setMaxProgLen(int NewLen);

	/* all pointer of population are setting of nil,
	* ATTENTION: of orphaned Object*/
	void flush ();


	/* take out the individual at the given position out the population, supply a pointer of this and 
	* dosen't reduced the size of the population */
	MT_Individual * removeIndividual(int Pos);
	
	/* take out the individual at the given position out the population, supply a pointer of this and 
	* reduced the size of the population */
	MT_Individual * delIndividual (int Pos);

	/* extended the population of one and add the given individual (by a pointer) at the end*/
	int addIndividual (MT_Individual *NewIndividual );

	/* insert the given individual at the given position,
	* return a pointer of the existing individual
	*/
	MT_Individual * insertAtPos(MT_Individual * NewIndividual , int Pos);
	
	/* insert the given individual at the the next free place, 
	* the return value is indicate the position of the individual in the population;
	* if the population is full, the return value is -1 */
	int insertIndividual (MT_Individual *NewIndividual);

	/* create a random population of the given size, delete all present individuals */
	void createNewPop(int PSize, MT_Randomizer *Randi);

	/* create random "NumberOfNewIndi" individuals, which added to the present population*/
	void createNewIndis(int NumberOfNewIndi, MT_Randomizer * Randi);

	/* create random one individual, which added to the present population*/
	int createNewIndi(MT_Randomizer * Randi);

	/* supply a pointer of the required individual */
	MT_Individual * getIndividual(int Pos);

	/* supply the max. population size, it's not the present number of individuals*/
	int getSize();

	/* import a population from a file and add to the existed population*/
	int importPop(QTextStream & File);

	//* export the whole population into a file*/
	int exportPop(QTextStream & File);

	/* load a population and write over the present population*/
	int loadPop(QTextStream & File);

	void writeToFilePop(QTextStream & File);
	
	MT_Population(QTextStream & File);
	/* construct a random population with n individuals*/
	MT_Population(MT_Randomizer *Randi, int n);
	MT_Population();
	virtual ~MT_Population();

protected:

	int LastError;
	bool Change;

	/*Indicate the first free position in the population
	* FirstFreePos<= PopSize !
	*FirstFreePos=PopSize indicate that the population is full
	*/
	int FirstFreePos;

	/* Indicate the max. Size of the Population*/
	int PopSize;

	/* Individuals is a QArray which include pointers of MT_Individuals, which represent the Metapopulation
	* Attention: the population dosen't completed filled !
	* If a Population represent the parents it's should be !
	*/
	Q2Array<MT_Individual*> Individuals;
};

#endif // !defined(AFX_MT_POPULATION_H__76BD10AD_932E_42FC_BDD3_85DD57B1D541__INCLUDED_)
