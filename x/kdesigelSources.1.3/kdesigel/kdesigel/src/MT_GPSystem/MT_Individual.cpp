// MT_Individual.cpp: Implementierung der Klasse MT_Individual.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GPSystem/MT_Individual.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

MT_Individual::MT_Individual()
{
	LastError=0;
	Name=-1;
	Age =0;
	Fitness =-1.0;
	Position =-1;
	TSetName=-1;

	FitnessOfParent =-1.0;
	TspOfGenesis = -1;

}

MT_Individual::~MT_Individual()
{

	delete Program;
}

MT_Individual::MT_Individual(int Na, MT_Randomizer *Randi)
{
	LastError=0;
	Name=Na;
	Age =0;
	Fitness =-1.0;
	Position =-1;
	TSetName=-1;
	FitnessOfParent =-1.0;
	TspOfGenesis = 0;

	Program = new MT_Program(Randi);


}

MT_Individual::MT_Individual(QTextStream &File)
{
	LastError=0;
	QString Individual( "Individual:" );
	QString PresentLine = File.readLine();
	while ((PresentLine != Individual) && !(File.atEnd()))
		PresentLine = File.readLine();
	if ((PresentLine == Individual) && !(File.atEnd()))
	{
		Name= (File.readLine()).toInt();
		Age = (File.readLine()).toInt();
		Fitness = (File.readLine()).toDouble();
		Position = (File.readLine()).toInt();
		TSetName= (File.readLine()).toInt();
		FitnessOfParent = (File.readLine()).toDouble();
		TspOfGenesis = (File.readLine()).toInt();

		Program = new MT_Program(File);
	}
	else
		LastError=370;
}

MT_Individual::MT_Individual(MT_Program *Prog, bool copy)
{
	LastError=0;
	Name=-1;
	Age =0;
	Fitness =-1.0;
	Position =-1;
	TSetName=-1;
	
	FitnessOfParent = -1.0;
	TspOfGenesis = 3;

	Program =0;
	
	if (copy == true)
	{
		Program = new MT_Program(Prog);
	}

	else
	{
		Program = Prog;
	}
	
}

void MT_Individual::writeToFileIndi(QTextStream &File)
{

	File << "Individual:\n";
	File << Name << endl;
	File << Age << endl;
	File << Fitness << endl;
	File << Position << endl;
	File << TSetName << endl;
	File << FitnessOfParent << endl;
	File << TspOfGenesis << endl;
	File << endl;
	
	if (Program !=NULL)
		Program->writeToFileProgram(File);
}

MT_Program * MT_Individual::getProgram()
{
	
	return Program;
}


MT_Programline* MT_Individual::getProgramLine(int Line)
{

	return Program->getProgramLine(Line);
}

int MT_Individual::getAge()
{

	return Age;
}

int MT_Individual::getName()
{

	return Name;
}

double MT_Individual::getFitness()
{

	return Fitness;
}

int MT_Individual::getPosition()
{

	return Position;
}

void MT_Individual::setPosition(int Pos)
{

	Position = Pos;

}

void MT_Individual::setFitness(double NewFitness)
{
	Fitness=NewFitness;

}

void MT_Individual::setTrainingsSet(int TName)
{
	TSetName=TName;
		
}

void MT_Individual::increaseAge()
{
	Age =Age +1;

}

bool MT_Individual::toBeEvaluated(int TSet)
{
	if (TSet == TSetName)
		return true;
	else 
		return false;

}

void MT_Individual::createProgram(MT_Randomizer *Randi)
{
	
	delete Program;
	Program = new MT_Program(Randi); 
	TspOfGenesis =0;
	FitnessOfParent= -1.0;
}

void MT_Individual::setMaxProgLen(int NewLen)
{

		Program->changeMaxProgLen(NewLen);
}

void MT_Individual::changeMaxNumVariable(int NewNum)
{

		Program->changeMaxNumVariable(NewNum);
}



double MT_Individual::getFitnessOfParent()
{
	return FitnessOfParent;
}

void MT_Individual::setFitnessOfParent(double FitOfParent)
{
	FitnessOfParent = FitOfParent;
}

int MT_Individual::getTypOfGenesis()
{
	return TspOfGenesis;
}

int MT_Individual::getLastError()
{
	return LastError;
}

void MT_Individual::setTypOfGenesis(int Typ)
{
	TspOfGenesis = Typ;
}


void MT_Individual::setNewProgram(MT_Program *NewProg)
{

	delete Program;
	Program =  new MT_Program(NewProg);


}

void MT_Individual::setNewAge(int NewAge)
{
	Age = NewAge;

}

void MT_Individual::setNewName(int NewName)
{
	Name = NewName;

}

QString MT_Individual::printProgramLine(int index)
{
	MT_Programline * Line = getProgramLine(index);
	
	QString LineAsString;

	MT_Instruction Instruction = Line->getInstruction();

	if (Instruction == swi)
		LineAsString = ("swt");
	if (Instruction == cop)
		LineAsString = ("cpy");
	if (Instruction == add)
		LineAsString = ("add");
	if (Instruction == sub)
		LineAsString = ("sub");
	if 	(Instruction == mul)
		LineAsString = ("mul");
	if (Instruction == mtd)
		LineAsString = ("div");
	if (Instruction == mod)
		LineAsString = ("mod");
	if (Instruction == mmin)
		LineAsString = ("min");
	if (Instruction == mmax)
		LineAsString = ("max");
	if (Instruction == cmp)
		LineAsString = ("cmp");
	if (Instruction == jmp)
		LineAsString = ("jmp");
	if (Instruction == jle)
		LineAsString = ("jle");
	if (Instruction == jeq)
		LineAsString = ("jeq");
	if (Instruction == nop)
		LineAsString = ("nop");
	if (Instruction == lui)
		LineAsString = ("lui");
	if (Instruction == lus)
		LineAsString = ("lus");
	if (Instruction == lud)
		LineAsString = ("lud");
	if (Instruction == lum)
		LineAsString = ("lum");

	
	LineAsString += " ";
	QString OpA;
	OpA.setNum((Line->getSourceOperand())->VariableName);
	LineAsString += OpA.leftJustify (5,' ', true);

	QString OpB;
	
	if ((Line->getTargetOperand())->OPType ==1)
	{
		OpB.setNum((Line->getTargetOperand())->VariableName);
		LineAsString += "  V ";
		LineAsString +=  OpB.leftJustify (5,' ', true);

	}
	else
	{
		OpB.setNum((Line->getTargetOperand())->Data);
		LineAsString += "  C ";
		LineAsString += OpB;

	}

	return (LineAsString);

}
