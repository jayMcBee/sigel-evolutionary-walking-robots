// MT_Search.cpp: implementation of class MT_Search.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GPSystem/MT_Search.h"
#include <qfile.h>

//////////////////////////////////////////////////////////////////////
// Construction/destruction
//////////////////////////////////////////////////////////////////////

MT_Search::MT_Search()
{

}

MT_Search::~MT_Search()
{

}

MT_Search::MT_Search(MT_Population *ParentPop, MT_Population *OffspringPop, MT_Randomizer *_Randi)
{
	LastError =0;
	TargetPop = OffspringPop;
	SourcePop = ParentPop;
	Randi = _Randi;
	ChildOne =0;
	ChildTwo =0;
	Parent=0;
	FirstXOverParent=0;

}

MT_Search::MT_Search(MT_Population *ParentPop, MT_Population *OffspringPop, MT_Randomizer *_Randi, QTextStream &File)
{
	LastError =0;
	TargetPop = OffspringPop;
	SourcePop = ParentPop;
	Randi = _Randi;
	
	ChildOne =0;
	ChildTwo =0;
	Parent=0;
	FirstXOverParent=0;

	QString BrutString ( "BrutSize:" );
	QString PresentLine = File.readLine(); 
	
	while ( (PresentLine != BrutString) && !(File.atEnd()))
		PresentLine = File.readLine();
	
	if ((PresentLine == BrutString) && !(File.atEnd()))
		BrutSize = (File.readLine()).toInt();
	else
		LastError =410;

}

int MT_Search::getBrutSize()
{

	return BrutSize; 
}

int MT_Search::getLastError()
{
	return LastError; 
}

void MT_Search::setBrutSize(int SizeOfBrut)
{
	BrutSize = SizeOfBrut;
}

MT_Individual * MT_Search::mutate(MT_Individual * Progenitor)
{
	
	MT_Individual * Child = (new MT_Individual(Progenitor->getProgram(),true));
	
	MT_Program * ChildProgram = Child->getProgram();
	
	int NumOfMutation =0;

	for(int i=0; i< ChildProgram->getLength(); i++)
	{
		if (Randi->answerMutateLine())
		{
			//Iterate over the program line
			for(int k=0; k<3; k++)
			{
				
				if (Randi->answerMutateElement())
				{
					NumOfMutation++;
					switch (k)
					{ 
					case 0 :
						{
							// Mutiere Befehl!
							(ChildProgram->getProgramLine(i))->setInstruction(Randi->getRandomInstruktion());
						} break;

					case 1:
						{
							// Mutiere Variable - 1.Operand
							(ChildProgram->getProgramLine(i))->setOperandA(Randi->getRandomVariable());
						
						} break;

					case 2:
						{
							// Mutiere 2. Operand!
							(ChildProgram->getProgramLine(i))->setOperandB(Randi->getRandomOperand());
						
						} break;
					}
				}
			}
		}
	}

	Child->setTypOfGenesis(100+NumOfMutation);

	return Child;

}



void MT_Search::crossover(MT_Individual * ParentOne, MT_Individual * ParentTwo)
{

/* ************************* Initialisationen  **************************/

	MT_Program * POneProgram = ParentOne->getProgram();
	MT_Program * PTwoProgram = ParentTwo->getProgram();

	int MaxLength = POneProgram->getMaxLength();

	QList<int> XPoints = *(Randi->getRandomXPoints(POneProgram->getLength(),PTwoProgram->getLength()));
	/* contain the position for a crossover Event
	* Array[0] = indicate the number of X points for this crossover event
	* Array[1] = the firste XPoint in the first Individual
	* Array[2] = the firste XPoint in the second Individual
	* Array[3] = the second XPoint in the first Individual
	* Array[4] = the second XPoint in the second Individual
	* Array[5] = the third XPoint in the first Individual
	* Array[6] = the third XPoint in the second Individual
	*/
	
/* ************************* Create new individuals, each with the first part **************************/

	MT_Program * ChildOneProgram = new MT_Program(POneProgram,0,(XPoints[1]-1));
	MT_Program * ChildTwoProgram = new MT_Program(PTwoProgram,0,(XPoints[2]-1));
	
	ChildOneProgram->changeMaxProgLen(MaxLength);
	ChildTwoProgram->changeMaxProgLen(MaxLength);

	ChildOne = new MT_Individual(ChildOneProgram, false);
	ChildTwo = new MT_Individual(ChildTwoProgram, false);


/* ************************* The remaining program fragment(s) are appended to ChildOne and ChildTwo **************************/

	MT_Program * NextProgPartForChildOne;
	MT_Program * NextProgPartForChildTwo;

	switch (XPoints[0])
	{

	// Only 1 crossover point was chosen
	case 0:
		{
			NextProgPartForChildOne = new MT_Program(PTwoProgram, XPoints[2], (PTwoProgram->getLength()-1));
			NextProgPartForChildTwo = new MT_Program(POneProgram, XPoints[1], (POneProgram->getLength()-1));
		
			ChildOneProgram->insertProg(NextProgPartForChildOne);
			ChildTwoProgram->insertProg(NextProgPartForChildTwo);
			
			delete NextProgPartForChildTwo;
			delete NextProgPartForChildOne;

			ChildOne->setTypOfGenesis(1);
			ChildTwo->setTypOfGenesis(1);

		} break;
	
	// 2 crossover points are used
	case 1: 
		{
			NextProgPartForChildOne = new MT_Program(PTwoProgram, XPoints[2], (XPoints[4]-1));
			NextProgPartForChildTwo = new MT_Program(POneProgram, XPoints[1], (XPoints[3]-1));
			
			ChildOneProgram->insertProg(NextProgPartForChildOne);
			ChildTwoProgram->insertProg(NextProgPartForChildTwo);
			
			delete NextProgPartForChildTwo;
			delete NextProgPartForChildOne;

			NextProgPartForChildOne = new MT_Program(POneProgram, XPoints[3], (POneProgram->getLength()-1));
			NextProgPartForChildTwo = new MT_Program(PTwoProgram, XPoints[4], (PTwoProgram->getLength()-1));
			
			ChildOneProgram->insertProg(NextProgPartForChildOne);
			ChildTwoProgram->insertProg(NextProgPartForChildTwo);
			
			delete NextProgPartForChildTwo;
			delete NextProgPartForChildOne;

			ChildOne->setTypOfGenesis(2);
			ChildTwo->setTypOfGenesis(2);

			
		} break;

	// 3 crossover points are used
	case 2:
		{
			NextProgPartForChildOne = new MT_Program(PTwoProgram, XPoints[2], (XPoints[4]-1));
			NextProgPartForChildTwo = new MT_Program(POneProgram, XPoints[1], (XPoints[3]-1));
			
			ChildOneProgram->insertProg(NextProgPartForChildOne);
			ChildTwoProgram->insertProg(NextProgPartForChildTwo);
			
			delete NextProgPartForChildTwo;
			delete NextProgPartForChildOne;

			NextProgPartForChildOne = new MT_Program(POneProgram, XPoints[3], (XPoints[5]-1));
			NextProgPartForChildTwo = new MT_Program(PTwoProgram, XPoints[4], (XPoints[6]-1));

			ChildOneProgram->insertProg(NextProgPartForChildOne);
			ChildTwoProgram->insertProg(NextProgPartForChildTwo);
						
			delete NextProgPartForChildTwo;
			delete NextProgPartForChildOne;

			NextProgPartForChildOne = new MT_Program(PTwoProgram, XPoints[6], (PTwoProgram->getLength()-1));
			NextProgPartForChildTwo = new MT_Program(POneProgram, XPoints[5], (POneProgram->getLength()-1));
			
			ChildOneProgram->insertProg(NextProgPartForChildOne);
			ChildTwoProgram->insertProg(NextProgPartForChildTwo);
			
			delete NextProgPartForChildTwo;
			delete NextProgPartForChildOne;

			ChildOne->setTypOfGenesis(3);
			ChildTwo->setTypOfGenesis(3);

		} break;

	}
}

MT_Individual*  MT_Search::reproduce(MT_Individual * Progenitor)
{

	return (new MT_Individual(Progenitor->getProgram(), true));

}

int MT_Search::startMatingProcess()
{

/* ************************* Initialisationen  **************************/
	int ParentSize = SourcePop->getSize();
	int OffspringSize = TargetPop->getSize();
	int FreePos =0; 
	int ParentPos = Randi->getRandomInteger(ParentSize); // the parent under consideration 

	ChildOne =0;
	ChildTwo =0;
	Parent=0;
	FirstXOverParent=0;

	bool CrossOver = false;

/* ************************* Copy parent pointers into the offspring **************************/
	TargetPop->flush();
	TargetPop->setFreePos(OffspringSize);
	
	int RandomPos =0;
	
	for(FreePos=0; FreePos<ParentSize; FreePos++)
	{
		Parent = SourcePop->getIndividual(FreePos);
		Parent->setTypOfGenesis(5);
//		Parent->setFitnessOfParent(Parent->getFitness());

		RandomPos = Randi->getRandomInteger(OffspringSize);
	
		while (TargetPop->getIndividual(RandomPos)!= 0)
		{
			RandomPos ++;
			if (RandomPos>=OffspringSize)
				RandomPos =0;
		}

		Parent->setPosition(RandomPos);
		TargetPop->insertAtPos(Parent,RandomPos);
	}
	
/* ************************** Insert the newly created individuals ****************** */
	for (FreePos=0;FreePos<OffspringSize;)
	{
		switch (Randi->getRandomSOperator())
		{
		
		/* Reproduction of the parent*/
		case 3 :
			{
				ChildOne= reproduce(SourcePop->getIndividual(ParentPos));
				ChildOne->setTypOfGenesis(4);
				ChildOne->setFitnessOfParent((SourcePop->getIndividual(ParentPos))->getFitness());
		
				while(TargetPop->getIndividual(FreePos)!= 0)
					FreePos++;
								
				if (FreePos < OffspringSize)
				{
					ChildOne->setPosition(FreePos);
					TargetPop->insertAtPos(ChildOne,FreePos);
					ChildOne = NULL;
					FreePos++;
				}

			} break;
		
		
		/* Mutation of the parent */
		case 2 :
			{
				ChildOne = mutate(SourcePop->getIndividual(ParentPos));
				ChildOne->setFitnessOfParent((SourcePop->getIndividual(ParentPos))->getFitness());
				
				while(TargetPop->getIndividual(FreePos)!= 0)
					FreePos++;
				
				if (FreePos < OffspringSize)
				{
					ChildOne->setPosition(FreePos);
					TargetPop->insertAtPos(ChildOne,FreePos);
					ChildOne = NULL;
					FreePos++;
				}

			} break;
		
		/*Recombination with the parent, if a "partner" is available */
		case 1 :
			{
				if (CrossOver==true)
				{
					crossover(FirstXOverParent, SourcePop->getIndividual(ParentPos));

					while(TargetPop->getIndividual(FreePos)!= 0)
						FreePos++;
					
					if (FreePos <OffspringSize)
					{
						ChildOne->setFitnessOfParent(FirstXOverParent->getFitness());
						ChildOne->setPosition(FreePos);
						TargetPop->insertAtPos(ChildOne,FreePos);
						ChildOne =0;
						FreePos++;
					}

					if (FreePos<OffspringSize)
					{
						while(TargetPop->getIndividual(FreePos)!= 0)
							FreePos++;
					
						if(FreePos<OffspringSize)
						{
							ChildTwo->setFitnessOfParent((SourcePop->getIndividual(ParentPos))->getFitness());
							ChildTwo->setPosition(FreePos);
							TargetPop->insertAtPos(ChildTwo,FreePos);
							ChildTwo =0;
							FreePos++;
						}
					
					}
				
					CrossOver = false;
				}
				else    // no crossover partner found so far
				{
					CrossOver = true;
					FirstXOverParent = SourcePop->getIndividual(ParentPos);
				}
			} break;
		} // end of switch
		
		ParentPos++;
		if (ParentPos == ParentSize)
			ParentPos =0;
	} // end of for over offspring

	if (ChildOne != NULL)
		delete ChildOne;
	if (ChildTwo != NULL)
		delete ChildTwo;

	SourcePop->flush();
		
	if (TargetPop->getLastError() !=0)
		return TargetPop->getLastError();

	return 0;


}



