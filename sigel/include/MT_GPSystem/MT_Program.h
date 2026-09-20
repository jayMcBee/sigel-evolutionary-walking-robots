// MT_Program.h: interface for class MT_Program.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_PROGRAM_H__D3578776_1315_4C75_B00A_E11B559B7314__INCLUDED_)
#define AFX_MT_PROGRAM_H__D3578776_1315_4C75_B00A_E11B559B7314__INCLUDED_

#include <QTextStream>
#include <qstring.h>
#include <qtextstream.h>
#include "MT_GPSystem/MT_Programline.h"
#include "MT_GPSystem/MT_Randomizer.h"	


/*
* This class are the main part of a META Individual. 
* It's the Program that represent a META Evaluator resp. the META Classifier.
*/

class MT_Program  
{
public:

	/* the constructor creating a new program from a programpart,
	* which is  between the Start and inclusive End of the given program
	* @ pre: Start and End are valid! Start and End must >= 0 and Start<=End
	* and End <=Prog->Lenght
	*/
	MT_Program(MT_Program *Prog, int Start, int End);
	
	/* the constructors for copy a given program*/
	MT_Program(MT_Program *Prog);

	/* the constructors for loading a program  */
	MT_Program(QTextStream &File );

	/* the constructors for creating a random program  */
	MT_Program(MT_Randomizer *Randi );
	
	MT_Program();

	virtual ~MT_Program();

	void writeToFileProgram(QTextStream &File);


	/* append the given program to the present program.
	* If the Lenght of the Program > MaxLen then the program will be reduced
	* and the excess programline will delete!
	* Attention: Part must (maybe) delete by the called function!
	* @post: the Part is emptily! that mean all program line are NULL;  
	*/
	void insertProg(MT_Program * Part);
	
	/* supply the program length*/
	int getLength();
	
	/* supply the max length*/
	int getMaxLength();

	/* supply a pointer of programLine (Index) */
	MT_Programline * getProgramLine (int Index);

	/* set all pointer of the program of NULL; it's used by insertProg*/
	void clearProgram();

	/* change the max Numbers of Variable, which can use by the Interpreter
	* if  a variable name > NewNum so it will set of (variable name) mod NewNum
	*@post: all variable names are between 0 and NewNum-1 ! 
	*/
	void changeMaxNumVariable(int NewNum);

	/* change the max Program lenght of the program, if the MaxLength reduce,
	* the excess programline will delete!
	*/
	void changeMaxProgLen(int NewLen);

	int getLastError();



private:
	int LastError;
	int MaxLength;
	int Length;
//	QVector<MT_Programline> Program;
	MT_Programline **Program;

	void resize(int nSize);
};

#endif // !defined(AFX_MT_PROGRAM_H__D3578776_1315_4C75_B00A_E11B559B7314__INCLUDED_)
