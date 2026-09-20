// MT_Operand.cpp: implementation of class MT_Operand.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GPSystem/MT_Operand.h"

//////////////////////////////////////////////////////////////////////
// Construction/destruction
//////////////////////////////////////////////////////////////////////

MT_Operand::MT_Operand()
{

	OPType=1;
	VariableName=0;
	Data =-1;
}

MT_Operand::~MT_Operand()
{

}

MT_Operand::MT_Operand(MT_Operand *Original)
{
	OPType = Original->OPType;
	VariableName= Original->VariableName;
	Data= Original->Data;
}

MT_Operand::MT_Operand(int Op, double Da, int na)
{

	OPType = Op;

	if (OPType == 1)  // if Operand is a variable 
	{
		VariableName = na;
		Data = -1;
	}
	else			// or Operand is a constant
	{ 
		Data= Da;
		VariableName = -1;
	}
	

}


