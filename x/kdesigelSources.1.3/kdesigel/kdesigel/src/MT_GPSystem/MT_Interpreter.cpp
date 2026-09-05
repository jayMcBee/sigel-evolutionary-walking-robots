// MT_Interpreter.cpp: implementation of class MT_Interpreter.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GPSystem/MT_Interpreter.h"

//////////////////////////////////////////////////////////////////////
// Construction/destruction
//////////////////////////////////////////////////////////////////////

MT_Interpreter::MT_Interpreter()
{

	Error =0;
	Program =0;
}

MT_Interpreter::~MT_Interpreter()
{

}

MT_Interpreter::MT_Interpreter(int VarNum, int TDuration)
{
	NumberVariable=VarNum;
	
	Variables.resize(NumberVariable);
	DurationToInterpret = TDuration;
	resetMachine();

	Error =1;
}

//////////////////////////////////////////////////////////////////////
// further methods
//////////////////////////////////////////////////////////////////////

void MT_Interpreter::resetMachine()
{
	
	for(int i=0; i<Variables.size(); i++)
		Variables[i]=0;

	StatusFlagOne =0;
	StatusFlagTwo =0;
	
	ProgramCounter=0;
}


int MT_Interpreter::getLastError()
{
	return Error;
}


double MT_Interpreter::interpret(MT_TranslatedIndividual *RobProg)
{
	// Source and Var serve as index for operand catching   
	int Source;
	int Var;
	// Mod1 and Mod2 serve for casting double in integer for e.g. mod calculation   
	int Mod1;
	int Mod2;
	// in the case of two robot control program, the Offset indicate which robot program
    // is use for examination 
	int Offset=0;
	// Con serve as interception a Constant   
	double Con;

	int NumOfInstruction = Program->getLength();
	if (NumOfInstruction ==0)
		return -10.0;

	int SigProLengthOne = RobProg->Boundary; 
	if (SigProLengthOne<0)
		SigProLengthOne = RobProg->T_length; // only one Sigel Program 

	int SigProLengthTwo = RobProg->T_length-SigProLengthOne; // 0 or Length of the second SigelProgram

	if (SigProLengthTwo <0)
		SigProLengthTwo =0;
			
	if (SigProLengthOne <0)
		SigProLengthOne =0;
	

	int StartPosOfTwo = 0;
	if (SigProLengthTwo !=0)
		StartPosOfTwo = SigProLengthOne;

	MT_Instruction PresentInstruction;
	MT_Programline * PresentLine;

	resetMachine();

// getSourceOperand and getTargetOperand are poorly named:
// getSourceOperand := first operand -> possibly also the target, or always? 
// getTargetOperand:= second operand  

	for (int i=0; i<DurationToInterpret; i++)
	{

		PresentLine = Program->getProgramLine(ProgramCounter);
		PresentInstruction = PresentLine->getInstruction();
		
		Source = (PresentLine->getSourceOperand())->VariableName;
		if ((Source >= Variables.size()) || (Source < 0))
		{
			int debug = Variables.size();

		}

		Var = (PresentLine->getTargetOperand())->VariableName;  
		if ( 1 ==((PresentLine->getTargetOperand())->OPType))
			if ((Var>=Variables.size())||(Var < 0))
				int debug = Variables.size();

		if ((ProgramCounter >= NumOfInstruction) || (ProgramCounter < 0))
			int debug =Program->getLength();




		switch (PresentInstruction)
		{

/* instruction to change the scanning SIGEL Robot by to individuals, */
		case swi :{
						if(RobProg->Boundary != -1)
						{
							if (Offset!= 0)
								Offset =0;
							else 
								Offset = StartPosOfTwo;
							// StartPosOfTwo is 0 if SigProLengthTwo == 0 --> only one SIGEL program in the training case --> NOP 
						}
						ProgramCounter++;
					 }break;


/* instruction for data transport: */
		case cop: { 
					Source = (PresentLine->getSourceOperand())->VariableName;
									
					if ( 1 ==((PresentLine->getTargetOperand())->OPType))
					{
						Var = (PresentLine ->getTargetOperand())->VariableName;
						Variables[Source] = Variables[Var];
					}
					else
					{
						Con = (PresentLine->getTargetOperand())->Data;
						Variables[Source] = Con;
					}

					ProgramCounter++;


				   } break;

/* arithmetical instruction*/
		case add : {
					Source = (PresentLine->getSourceOperand())->VariableName;
									
					if ( 1 ==((PresentLine->getTargetOperand())->OPType))
					{
						Var = (PresentLine->getTargetOperand())->VariableName;
						Variables[Source] =Variables[Source]+Variables[Var];
					}
					else
					{
						Con = (PresentLine->getTargetOperand())->Data;
						Variables[Source] =Variables[Source]+Con;
					}

					ProgramCounter++;
				   } break;

		case sub : {
					Source = (PresentLine->getSourceOperand())->VariableName;
									
					if ( 1 ==((PresentLine->getTargetOperand())->OPType))
					{
						Var = (PresentLine->getTargetOperand())->VariableName;
						Variables[Source] =Variables[Source]-Variables[Var];
					}
					else
					{
						Con = (PresentLine->getTargetOperand())->Data;
						Variables[Source] =Variables[Source]-Con;
					}

					ProgramCounter++;

				   } break;

		case mul : {
					Source = (PresentLine->getSourceOperand())->VariableName;
									
					if ( 1 ==((PresentLine->getTargetOperand())->OPType))
					{
						Var = (PresentLine->getTargetOperand())->VariableName;
						Variables[Source] =Variables[Source]*Variables[Var];
					}
					else
					{
						Con = (PresentLine->getTargetOperand())->Data;
						Variables[Source] =Variables[Source]*Con;
					}

					ProgramCounter++;

				   } break;

		case mtd : { 
					// div by Zero, will be ignore
					Source = (PresentLine->getSourceOperand())->VariableName;
									
					if ( 1 ==((PresentLine->getTargetOperand())->OPType))
					{
						Var = (PresentLine->getTargetOperand())->VariableName;
						if (Variables[Var] != 0.0)
							Variables[Source] =Variables[Source]/Variables[Var];
					}
					else
					{
						Con = (PresentLine->getTargetOperand())->Data;
						if (Con != 0.0)
							Variables[Source] =Variables[Source]/Con;
					}

					ProgramCounter++;

				   } break;

		case mod : { 
					// mod by Zero, will be ignore
					Source = (PresentLine->getSourceOperand())->VariableName;
									
					if ( 1 ==((PresentLine->getTargetOperand())->OPType))
					{
						Var = (PresentLine->getTargetOperand())->VariableName;
						Mod2=Variables[Var];

						if (Mod2 != 0)
						{
							Mod1=Variables[Source];
							
							if(Mod2 == -1)
								if (Mod1 == -2147483648)
									Mod2 =1;
					
							Variables[Source] =Mod1%Mod2;
						}
					}
					else
					{
						Mod2 = (PresentLine->getTargetOperand())->Data;
						if(Mod2!=0)
						{

							Mod1=Variables[Source];
													
							if(Mod2 == -1)
								if (Mod1 == -2147483648)
									Mod2 =1;
						
							Variables[Source] =Mod1%Mod2;
						}
					}

					ProgramCounter++;
				   } break;

		case mmin : {
					Source = (PresentLine ->getSourceOperand())->VariableName;
									
					if ( 1 ==((PresentLine->getTargetOperand())->OPType))
					{
						Var = (PresentLine->getTargetOperand())->VariableName;
						if (Variables[Var]<Variables[Source])
							Variables[Source] =Variables[Var];
					}
					else
					{
						Con = (PresentLine->getTargetOperand())->Data;
						if(Con<Variables[Source])
							Variables[Source]= Con;
					}

					ProgramCounter++;
				   } break;

		case mmax : {
					Source = (PresentLine->getSourceOperand())->VariableName;
									
					if ( 1 ==((PresentLine->getTargetOperand())->OPType))
					{
						Var = (PresentLine->getTargetOperand())->VariableName;
						if (Variables[Var]>Variables[Source])
							Variables[Source] =Variables[Var];
					}
					else
					{
						Con = (PresentLine->getTargetOperand())->Data;
						if(Con>Variables[Source])
							Variables[Source]= Con;
					}

					ProgramCounter++;
				   } break;

/* program control instruction*/
		case cmp : {
					Source = (PresentLine ->getSourceOperand())->VariableName;
									
					if ( 1 ==((PresentLine ->getTargetOperand())->OPType))
					{
						Var = (PresentLine ->getTargetOperand())->VariableName;
						if (Variables[Source] <= Variables[Var])
						{
							if (Variables[Source] == Variables[Var])
								StatusFlagOne = 1;
							else
								StatusFlagOne = 2;
						}
						else
							StatusFlagOne = 0;

					}
					else
					{
						Con = (PresentLine ->getTargetOperand())->Data;
						if (Variables[Source] <= Con)
						{
							if (Variables[Source] == Con)
								StatusFlagOne = 1;
							else
								StatusFlagOne = 2;
						}
						else
							StatusFlagOne = 0;
					}

					ProgramCounter++;
				   } break;

		case jmp : {
					Source = (PresentLine ->getSourceOperand())->VariableName;
					Mod1= Variables[Source];
					ProgramCounter = Mod1;

					if (ProgramCounter<0){
						ProgramCounter =ProgramCounter * -1;
						if (ProgramCounter == -2147483648)
							ProgramCounter =0;
					}
					if (ProgramCounter>=NumOfInstruction)
						ProgramCounter = ProgramCounter%NumOfInstruction;

					if ((ProgramCounter >= NumOfInstruction) || (ProgramCounter < 0))
						int debug =Program->getLength();

					} break;

		case jeq : {
						if (StatusFlagOne ==1)
						{
							Source = (PresentLine ->getSourceOperand())->VariableName;
							Mod1= Variables[Source];
							ProgramCounter = Mod1;

							if (ProgramCounter<0){
								ProgramCounter =ProgramCounter * -1;
									if (ProgramCounter == -2147483648)
									ProgramCounter =0;
							}

							if (ProgramCounter>=NumOfInstruction)
								ProgramCounter = ProgramCounter%NumOfInstruction;
							
						if ((ProgramCounter >= NumOfInstruction) || (ProgramCounter < 0))
							int debug =Program->getLength();

						}
						else
							ProgramCounter++;

					   } break;

		case jle : {
						if (StatusFlagOne ==2)
						{
							Source = (PresentLine ->getSourceOperand())->VariableName;
							Mod1= Variables[Source];
							ProgramCounter = Mod1;

							if (ProgramCounter<0){
								ProgramCounter =ProgramCounter * -1;
								if (ProgramCounter == -2147483648)
									ProgramCounter =0;
							}

							if (ProgramCounter>=NumOfInstruction)
								ProgramCounter = ProgramCounter%NumOfInstruction;
							
							if ((ProgramCounter >= NumOfInstruction) || (ProgramCounter < 0))
								int debug =Program->getLength();

						}
						else
							ProgramCounter++;
							

				   } break;


/* further instruction */
		case nop : { 
					ProgramCounter++;
				   } break;

/* instruction for interaction with the SIGEL robot program  */

		case lui : {
						Source = (PresentLine ->getSourceOperand())->VariableName;
									
						if ( 1 ==((PresentLine ->getTargetOperand())->OPType))
						{
							Var = (PresentLine ->getTargetOperand())->VariableName;
																	
							if (Offset==0)												
								Mod1 = abs( ((int) Variables[Var]) % SigProLengthOne );
							else 																
								Mod1 = Offset + abs( ((int)Variables[Var]) % SigProLengthTwo);

if ((Mod1 >= RobProg->T_length)||(Mod1<0))
	int Deb = RobProg->T_length;

							
							Variables[Source] = RobProg->T_Instruktion->at(Mod1);
																
						}
						else
						{																					
							if (Offset==0)
								Mod1 = abs(  ((int)(PresentLine->getTargetOperand()->Data)) % SigProLengthOne);
							else
								Mod1 = Offset + abs( ((int)(PresentLine->getTargetOperand()->Data)) % SigProLengthTwo);
							
if ((Mod1 >= RobProg->T_length)||(Mod1<0))
	int Deb = RobProg->T_length;

							Variables[Source] = RobProg->T_Instruktion->at(Mod1);	
						}

						ProgramCounter++;
					} break;

		case lud  : { 
						Source = (PresentLine ->getSourceOperand())->VariableName;
		
						if ( 1 ==((PresentLine ->getTargetOperand())->OPType))
						{
							Var = (PresentLine->getTargetOperand())->VariableName;

							if (Offset==0)								
								Mod1 = abs(((int)Variables[Var])%(SigProLengthOne));
							else 
								Mod1 = Offset + abs(((int)Variables[Var])%(SigProLengthTwo));
							
if ((Mod1 >= RobProg->T_length)||(Mod1<0))
	int Deb = RobProg->T_length;

							Variables[Source] = RobProg->T_Operand1->at(Mod1);
						}
						else
						{
							if (Offset==0)
								Mod1 = abs( ((int)(PresentLine->getTargetOperand()->Data)) % SigProLengthOne );
							else
								Mod1 = Offset + abs( ((int)(PresentLine->getTargetOperand()->Data)) % SigProLengthTwo);

if ((Mod1 >= RobProg->T_length)||(Mod1<0))
	int Deb = RobProg->T_length;

							Variables[Source] = RobProg->T_Operand1->at(Mod1);	
						}

						ProgramCounter++;

					} break;

		case lus : {	
							Source = (PresentLine ->getSourceOperand())->VariableName;
									
							if ( 1 ==((PresentLine ->getTargetOperand())->OPType))
							{
								Var = (PresentLine ->getTargetOperand())->VariableName;
														
								if (Offset==0)					
									Mod1 = abs(((int)Variables[Var])% SigProLengthOne);
								else 
									Mod1 = Offset + abs( ((int)Variables[Var])% SigProLengthTwo );

if ((Mod1 >= RobProg->T_length)||(Mod1<0))
	int Deb = RobProg->T_length;

								Variables[Source] = RobProg->T_Operand2->at(Mod1);
							}
							else
							{															
								if (Offset==0)
									Mod1 = abs( ((int)(PresentLine ->getTargetOperand()->Data))% SigProLengthOne);
								else
									Mod1 = Offset + abs( ((int)(PresentLine->getTargetOperand()->Data)) % SigProLengthTwo);

if ((Mod1 >= RobProg->T_length)||(Mod1<0))
	int Deb = RobProg->T_length;

								Variables[Source] = RobProg->T_Operand2->at(Mod1);
							}

						ProgramCounter++;

					} break;
// look up metadata
// 
	case lum : {
						Source = (PresentLine ->getSourceOperand())->VariableName;
									
						if (1 == ((PresentLine ->getTargetOperand())->OPType))
						{
							Var = (PresentLine ->getTargetOperand())->VariableName;

							Mod1 = abs((int)Variables[Var]);
							Mod1 = Mod1 % 16;  // 0 =< Mod1 <16

							if (Offset !=0)
								Mod1 = 16 + Mod1;  // 16 =< Mod1 <32

if ((Mod1 >= RobProg->MetaData->size())||(Mod1<0))
	int Deb = RobProg->T_length;

							Variables[Source] = RobProg->MetaData->at(Mod1);
																
						}
						else
						{	
						
							Mod1 = abs ((int)(PresentLine->getTargetOperand()->Data));
							Mod1 = Mod1 % 16;  // 0 =< Mod1 <16

							if (Offset !=0)
								Mod1 = 16 + Mod1;  // 16 =< Mod1 <32

if ((Mod1 >= RobProg->MetaData->size())||(Mod1<0))
	int Deb = RobProg->T_length;

							Variables[Source] = RobProg->MetaData->at(Mod1);	
						}

						ProgramCounter++;
					} break;


		}
		
		if (ProgramCounter>= NumOfInstruction)
			i = DurationToInterpret;		// End of program - the loop can be exited
		
	}

	return Variables[0]; 
}


int MT_Interpreter::loadProgram(MT_Program *Prog)
{
	Program = Prog;
	return Error;
}

void MT_Interpreter::setDuration(int TDuration)
{
	DurationToInterpret=TDuration;
}

void MT_Interpreter::setVariableNumber(int NumVariable)
{
	NumberVariable=NumVariable;
	Variables.resize(NumberVariable);
	resetMachine();
}

int MT_Interpreter::getDuration()
{

	return DurationToInterpret;
}



int MT_Interpreter::getVariableNumber()
{
	return NumberVariable;

}
