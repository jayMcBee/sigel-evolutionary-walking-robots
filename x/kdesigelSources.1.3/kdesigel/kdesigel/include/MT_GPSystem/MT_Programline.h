// MT_Programline.h: interface for class MT_Programline.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_PROGRAMLINE_H__09232832_11E1_4EEB_9D5A_435DD5D9826A__INCLUDED_)
#define AFX_MT_PROGRAMLINE_H__09232832_11E1_4EEB_9D5A_435DD5D9826A__INCLUDED_

#include "MT_GPSystem/MT_Instruction.h"
#include "MT_GPSystem/MT_Operand.h"
#include "MT_GPSystem/MT_Randomizer.h"

#include <qstring.h>
#include <qtextstream.h>



/*
* This class represent a Programline of the META Evaluator resp. the META Classifier.
* It's contain of an instruction and two operand. 
*/

class MT_Programline  
{
public:
	void setOperandB(MT_Operand *NewOpB);
	void setOperandA(MT_Operand * NewOpA);
	void setInstruction(MT_Instruction NewInstruc);

	
	/* change the max Numbers of Variable, which can use by the Interpreter
	* if  a variable name > NewNum so it will set of (variable name) mod NewNum
	*@post: all variable names are between 0 and NewNum-1 ! */
	void changeMaxNumVariable(int NewNum);

	/*
	* three get function for getting the attribute of programline,
	* Attention: the Source and the Target(better: second) Operand are given by Pointers 
	*/
	MT_Operand * getTargetOperand();
	MT_Operand * getSourceOperand();
	MT_Instruction getInstruction();

	
	/* This function writes an individual to a QTextStream.
	*/ 
	void writeToFileProgramLine (QTextStream &File);

	/*
	* four constructors for copy a given program line, for creating a random program line, 
	* for load a program line  and a standard constructor
	*/
	MT_Programline(MT_Programline *Line);
	MT_Programline (const QString &File);
	MT_Programline (MT_Randomizer *Randi);
	MT_Programline();

	virtual ~MT_Programline();

private:
	
	MT_Instruction Instruction;

	// the first Operand
	MT_Operand * OperandA;
	// the second Operand
	MT_Operand * OperandB;

};

#endif // !defined(AFX_MT_PROGRAMLINE_H__09232832_11E1_4EEB_9D5A_435DD5D9826A__INCLUDED_)
