/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler

  This file is part of Sigel.

  Sigel is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Sigel is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Sigel; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#ifndef MT_GPSYSTEM_MT_PROGRAMLINE_H
#define MT_GPSYSTEM_MT_PROGRAMLINE_H

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
	MT_Programline() = default;

	virtual ~MT_Programline();

private:
	
	MT_Instruction Instruction;

	// the first Operand
	MT_Operand * OperandA;
	// the second Operand
	MT_Operand * OperandB;
};

#endif // MT_GPSYSTEM_MT_PROGRAMLINE_H
