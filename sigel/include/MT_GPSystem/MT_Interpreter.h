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
#ifndef MT_GPSYSTEM_MT_INTERPRETER_H
#define MT_GPSYSTEM_MT_INTERPRETER_H

#include <QList>
#include "MT_GPSystem/MT_TranslatedIndividual.h"
#include "MT_GPSystem/MT_Program.h"
#include "MT_GPSystem/MT_ErrorCode.h"

/* This class interpret the MT_Program of the SIGLE translated Individual
* (the robot control program). 
*/
class MT_Interpreter  
{
private:
	MT_ErrorCode Error;
	/* Variables is a array of double with Size NumberVariable. 
	* It's represent the variable/ register which can use a MT_Program
	*/
	QList<double> Variables;

	int NumberVariable;
	
	/* indicate by a pointer the MT_Program, which should be interpret */ 
	MT_Program * Program;

	/* DurationToInterpret denote the Number of instruction, that can be done by a MT_Program*/
	int DurationToInterpret;

	/* StatusFlagOne and StatusFlagTwo serve for the interpretation of a MT_Program */
	int StatusFlagOne; //:=1 if last cmp A B -> A=B ; :=2 if last cmp A B -> A<=B; else 0 
	int StatusFlagTwo;

	/* count the Number of instruction, which have done */
	int ProgramCounter;

	/* Converts a variable to int; NaN and values outside int give INT_MIN, as x86 does. */
	int toInt(double value) const;

public:
	int getVariableNumber();

	/* the constructor receive the VariableNumber and the training duration 
	*/
	MT_Interpreter (int VarNum, int TDuration);

	MT_Interpreter();
	virtual ~MT_Interpreter() = default;
	
	/* this both function change the TDuration and the Number of allowed Variable, 
	* e.g. by a change in the GUI or load a new setup.
	* It's maybe necessary to resize the array of variables.
	*/
	void setVariableNumber(int NumVariable);
	void setDuration(int TDuration);

	/* supply the Duration for interpret, which is allowed,
	* e.g. for update the GUI after load a setup
	*/ 
	int getDuration();

	/* load a given MT_Program (by a pointer) to the Interpreter 
	* If an error occurs, the return value is not MT_ErrorCode::None.
	*/ 
	MT_ErrorCode loadProgram (MT_Program * Prog);

	/* interpret perform the MT_Program of the given translated SIGEL Individual
	* (the robot control program).
	* @pre: there are a loaded and correct MT_Program;
	* @pre: the all (!) VariableNames from the Program < NumberVariable !
	* @pre: the first Operand of the Programline is a variable !
	* @post: the output is the fitness of the robot control program
	* If an error occurs, the return value is negative.
	*/
	double interpret(MT_TranslatedIndividual * RobPro);

	/* If an error occurred, this function supplies a reason
	*	MT_ErrorCode::None indicate, that no error occurred
	*/
	MT_ErrorCode getLastError();

	/* This function reset the Interpret to the start conditions,
	* @post: the Flag's, instruction counter and the Variables are setting of zero
	*/
	void resetMachine();
};

#endif // MT_GPSYSTEM_MT_INTERPRETER_H
