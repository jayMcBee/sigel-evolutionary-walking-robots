// MT_Operand.h: Schnittstelle für die Klasse MT_Operand.
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
	
	/*OpTyp indicate that the Operand is a Variable (Optyp =1) 
	or that the Operand is a Constant (Optyp 2).*/
	int OPType;
    /* If Optyp= 1 then Variable represent the VariableName*/
	int VariableName;
	/* If Optyp= 2 then Data represent a constant */
	double Data;
	
	MT_Operand(int Op, double Da, int na);
	MT_Operand();
	virtual ~MT_Operand();

};

#endif // !defined(AFX_MT_OPERAND_H__BE22A5C1_39CA_4C2C_AC08_DE24728F3891__INCLUDED_)
