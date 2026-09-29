// MT_Operand.h: interface for class MT_Operand.
//
//////////////////////////////////////////////////////////////////////

#ifndef MT_GPSYSTEM_MT_OPERAND_H
#define MT_GPSYSTEM_MT_OPERAND_H




/*This class represent a Operand in a MT_Program. 
* Attention: a Operand is a variable xor a constant.
 */
class MT_Operand  
{
public:
	MT_Operand(MT_Operand * Original);
	
	/*OPType indicate that the Operand is a Variable (OPType =1) 
	or that the Operand is a Constant (OPType 2).*/
	int OPType;
    /* If OPType= 1 then Variable represent the VariableName*/
	int VariableName;
	/* If OPType= 2 then Data represent a constant */
	double Data;
	
	MT_Operand(int Op, double Da, int na);
	MT_Operand();
	virtual ~MT_Operand() = default;

};

#endif // MT_GPSYSTEM_MT_OPERAND_H
