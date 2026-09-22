// MT_Operand.h: interface for class MT_Operand.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MT_OPERAND_H__BE22A5C1_39CA_4C2C_AC08_DE24728F3891__INCLUDED_)
#define AFX_MT_OPERAND_H__BE22A5C1_39CA_4C2C_AC08_DE24728F3891__INCLUDED_




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
	virtual ~MT_Operand();

};

#endif // !defined(AFX_MT_OPERAND_H__BE22A5C1_39CA_4C2C_AC08_DE24728F3891__INCLUDED_)
