// MT_Programline.cpp: implementation of class MT_Programline.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GPSystem/MT_Programline.h"

//////////////////////////////////////////////////////////////////////
// Construction/destruction
//////////////////////////////////////////////////////////////////////

MT_Programline::MT_Programline()
{

}

MT_Programline::~MT_Programline()
{
	delete OperandA;
	delete OperandB;

}

MT_Programline::MT_Programline(MT_Randomizer *Randi)
{

	Instruction = Randi->getRandomInstruktion();
	OperandA = Randi->getRandomVariable();
	OperandB = Randi->getRandomOperand();

}

MT_Programline::MT_Programline(QString & File)
{
	QString PartOfLine = File.mid(0,3);

	if (PartOfLine == "swi" || PartOfLine == "swt")
		Instruction = swi;
	if (PartOfLine == "cop" || PartOfLine == "cpy")
		Instruction = cop;
	if (PartOfLine == "add")
		Instruction = add;
	if (PartOfLine == "sub")
		Instruction = sub;
	if (PartOfLine == "mul")
		Instruction = mul;
	if (PartOfLine == "mtd" || PartOfLine == "div")
		Instruction = mtd;
	if (PartOfLine == "mod")
		Instruction = mod;
	if (PartOfLine == "min")
		Instruction = mmin;
	if (PartOfLine == "max")
		Instruction = mmax;
	if (PartOfLine == "cmp")
		Instruction = cmp;
	if (PartOfLine == "jmp")
		Instruction = jmp;
	if (PartOfLine == "jle")
		Instruction = jle;
	if (PartOfLine == "jeq")
		Instruction = jeq;
	if (PartOfLine == "nop")
		Instruction = nop;
	if (PartOfLine == "lui")
		Instruction = lui;
	if (PartOfLine == "lus")
		Instruction = lus;
	if (PartOfLine == "lud")
		Instruction = lud;
	if (PartOfLine == "lum")
		Instruction = lum;

	// The first operand is always a variable
	PartOfLine = File.mid(4,5);
	OperandA = new MT_Operand(1,-1,PartOfLine.toInt());
	
	// 2ter Operand ? Variable ==1 sonst Konstante !
	PartOfLine = File.mid(9,2);
	if (PartOfLine =="V ")
	{
		PartOfLine = File.mid(11,5);
		OperandB = new MT_Operand (1,-1,PartOfLine.toInt());
	}
	else
	{
		PartOfLine = File.mid(11,20);
		OperandB = new MT_Operand(0,PartOfLine.toDouble(),0);
	}

}

MT_Programline::MT_Programline(MT_Programline *Line)
{

	 Instruction = Line->getInstruction();
	 OperandA = new MT_Operand(Line->getSourceOperand());
	 OperandB = new MT_Operand(Line->getTargetOperand());


}

void MT_Programline::writeToFileProgramLine(QTextStream &File)
{

	// Aufbau Programmzeile Beispiel"
	//"swp 4    V 5"  or  "cop 1245 C 12.34565567"

	if (Instruction == swi)
			File <<"swt";
	if (Instruction == cop)
			File << "cpy";
	if (Instruction == add)
			File << "add";
	if (Instruction == sub)
			File << "sub";
	if 	(Instruction == mul)
			File << "mul";
	if (Instruction == mtd)
			File <<"div";
	if (Instruction == mod)
			File << "mod";
	if (Instruction == mmin)
			File << "min";
	if (Instruction == mmax)
			File << "max";
	if (Instruction == cmp)
			File << "cmp";
	if (Instruction == jmp)
			File << "jmp";
	if (Instruction == jle)
			File << "jle";
	if (Instruction == jeq)
			File <<"jeq";
	if (Instruction == nop)
			File << "nop";
	if (Instruction == lui)
			File << "lui";
	if (Instruction == lus)
			File << "lus";
	if (Instruction == lud)
			File << "lud";
	if (Instruction == lum)
			File << "lum";


	File <<" ";

	QString OpA;
 	OpA.setNum(OperandA->VariableName);
	OpA = OpA.leftJustify (5,' ', true);
	File << OpA;

	QString OpB;
	
	if (OperandB->OPType ==1)
	{
		OpB.setNum(OperandB->VariableName);
		OpB = OpB.leftJustify (5,' ', true);
		File <<"V " << OpB << endl;

	}
	else
	{
		OpB.setNum(OperandB->Data);
		File <<"C " << OpB << endl;
	}

}

MT_Instruction MT_Programline::getInstruction()
{

	return Instruction;
}

MT_Operand * MT_Programline::getSourceOperand()
{

	return OperandA;
}

MT_Operand * MT_Programline::getTargetOperand()
{

	return OperandB;
}

void MT_Programline::changeMaxNumVariable(int NewNum)
{
	if (NewNum<2)
		NewNum = 2;

	if (OperandA->VariableName>=NewNum)
		OperandA->VariableName = OperandA->VariableName % NewNum;
	
	if ((OperandB->OPType ==1)&& (OperandB->VariableName>NewNum))
		OperandB->VariableName = OperandB->VariableName % NewNum;


}

void MT_Programline::setInstruction(MT_Instruction NewInstruc)
{
	Instruction = NewInstruc;

}

void MT_Programline::setOperandA(MT_Operand *NewOpA)
{
	
	delete OperandA;
	OperandA = NewOpA;
}

void MT_Programline::setOperandB(MT_Operand *NewOpB)
{
	delete OperandB;
	OperandB = NewOpB;
}
