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
/* ----------------------------------------------------------------------- */
/* CLASS:     SIG_ProgrammLine.h                                           */
/* FUNCTION:  This class contains all functions that are needed to work    */
/*            with a program line (that's contained by a SIG_Program).     */
/*            Every program line is a sequence of numbers, which are       */
/*            depended on the line's type.                                 */
/* ----------------------------------------------------------------------- */

#if !defined(SIGEL_PROGRAMPACKAGE_SIG_PROGRAMLINE_H)
#define SIGEL_PROGRAMPACKAGE_SIG_PROGRAMLINE_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QTextStream>

#include "SIGEL_Tools/SIG_Randomizer.h"
#include "SIGEL_Robot/SIG_LanguageParameters.h"

namespace SIGEL_Program
{

	/**
	 * Add a new instruction at the end only: other code hard-codes these values as indices, so the order must not change.
	 */
	enum RobotInstruction
	{
		COPY,
		LOAD,
		ADD,
		SUB,
		MUL,
		DIV,
		MOD,
		MIN,
		MAX,
		CMP,
		JMP,
		SENSE,
		MOVE,
		DELAY,
		NOP,
	};

	/**
	 * The names of the robot instructions, in the order of RobotInstruction.
	 */
	inline const QStringList robotInstructionNames = { "COPY", "LOAD", "ADD", "SUB", "MUL", "DIV", "MOD", "MIN", "MAX", "CMP", "JMP", "SENSE", "MOVE", "DELAY", "NOP" };

	inline const int instructionCount = robotInstructionNames.size();

	/**
	 * This class represents a program line that contains robot instructions (and more).
	 * A program line is a set of elements. The meaning of the elements is dependend on
	 * the type of an instruction. Every instruction type defines a fix meaning for
	 * every element.
	 */

	class SIG_ProgramLine
	{
	public:
		/**
		 * This constructor initializes an empty program line that does not contain any elements.
		 * @post
		 * An empty line that does not contain any data will be created.
		 */
		SIG_ProgramLine();

		/**
		 * This constructor generates a random instruction that is dependend on the language
		 * parameters. The probability to be chosen randomly is dependend on the given probabilities (prob).
		 */
		SIG_ProgramLine( SIGEL_Tools::SIG_Randomizer &randomizer,
		                 SIGEL_Robot::SIG_LanguageParameters &languageParameters,
		                 QList< int > &prob );

		/**
		 * An empty destructor.
		 * @post
		 * All data is destructed.
		 */
		virtual ~SIG_ProgramLine() = default;

		/**
		 * This function generates a random instruction that is dependend on the language
		 * parameters. The probability to be chosen randomly is dependend on the given probabilities (prob).
		 */
		void generateRandomRobotInstruction( SIGEL_Robot::SIG_LanguageParameters &languageParameters,
		                                     SIGEL_Tools::SIG_Randomizer &randomizer,
		                                     QList< int > &prob  );

		/**
		 * This function deletes the data of a program line completely.
		 * @post
		 * The program line is deleted completely.
		 */
		void clearLine();

		/**
		 * This instruction copies the program line source into the line destination.
		 * @pre
		 * The lines source and destination have to exist.
		 * @post
		 * The content of source has been copied into destination. WARNING: The content of destination
		 * will be deleted before the copy process starts.
		 */
		void copyLine( SIGEL_Program::SIG_ProgramLine *source,
		               SIGEL_Program::SIG_ProgramLine *destination );

		/**
		 * This function generates randomly a robot instruction. The choice is depended on the
		 * current language parameters and the given probabilities (prob).
		 */
		void randomRobotInstruction( SIGEL_Robot::SIG_LanguageParameters &languageParameters,
		                             SIGEL_Tools::SIG_Randomizer &randomizer,
		                             QList< int > &prob );

		/**
		 * This function is only for test purposes and prints a program line.
		 */
		void print() const;

		/**
		 * This function sets an element (with index no) to the value. value has to be a pre-defined
		 * constant value.
		 * @post
		 * The element (with index no) is set to value.
		 */
		void setElement( int no, int value );

		/**
		 * This function changes to size of the array that saves the elements.
		 * @post
		 * The size of the array will be changed. WARNING: Values (above the new size) are
		 * deleted (and lost).
		 */
		void resizeElements( int size );

		/**
		 * This function writes a program line into a QTextStream.
		 * @post
		 * The current program line will be added to the given QTextStream.
		 */
		void writeToFile( QTextStream &file ) const;

		/**
		 * Reads the program line in str into line and returns true. The former content of line is replaced.
		 * Returns false and ignores str if it holds no robot instruction, so a program can have comments.
		 * A comment must take a complete line and must not contain an instruction name, for example ADD.
		 * Throws a SIG_Exception if an operand is missing or is not an integer.
		 * @pre
		 * str holds one complete program line, written by writeToFile or edited by the user,
		 * in the format that the specification describes.
		 */
		bool readFromFile( QString &str,
		                   SIGEL_Program::SIG_ProgramLine *line );

		/**
		 * This function prepares a QString. This QString will contain the current program line.
		 * @post
		 * The QString will contain the current program line.
		 */
		void printToString(QString &lineStr) const;

		/**
		 * This function returns the array of a line's elements
		 */
		QList< int > getElementsArray() const;

		/**
		 * This function returns the type of the current robot instruction.
		 */
		RobotInstruction getRobotInstructionType() const;

		/**
		 * This function returns the element of the current robot instruction.
		 * The first element, e.g. a register, is to be accessed with index 0.
		 * In case of an invalid access, e.g. a not defined index, this function returns 0.
		 * Example 1: LOAD 1,432 -> getInstructionElement(0)=1,getInstructionElement(1)=432;
		 * Example 2: ADD 1,2 -> getInstructionElement(0)=1,getInstructionElement(1)=2;
		 * @return
		 * This function returns (in case of a valid access) the element with the index no.
		 */
		int getInstructionElement(int no) const;

		/**
		 * This function returns the element with the index no.
		 * @pre
		 * The index must be smaller than the number of elements.
		 * @return
		 * This function returns the element with the index no (in case of a correct index)
		 */
		int getElement(int no) const;

		/**
		 * This function gives an inormation about the number of elements at all.
		 * @return
		 * The number of elements will be returned.
		 */
		int getNumberOfElements() const;

		/**
		 * This operator copies a complete program line.
		 * @post
		 * The current program line will be copied completely.
		 */
		SIGEL_Program::SIG_ProgramLine& operator =(SIGEL_Program::SIG_ProgramLine const & prgLine);

		/**
		 * Sets the robot instrction type, e.g. SIGEL_Program::COPY, ...
		 */
		void setRobotInstructionType( RobotInstruction instr );

		/**
		 * A robot instruction can be set explicitely by using this function. The robot instruction
		 * is set to instructionType. The operands are defined by op1 and op2. If an instruction
		 * only needs one operad (op1) the second operand (op2) will be ignored.
		 */
		void setRobotInstruction(SIGEL_Program::RobotInstruction instr, int op1, int op2);

	protected:
		/**
		 * Internal function, only internal usage.
		 */
		void readTwoRegisters(QString &str,int &reg1,int &reg2);

		/**
		 * Internal function, only internal usage.
		 */
		void readOneRegisterAndIntegerConstant(QString &str,int &reg1,int &cnst);

		/**
		 * Internal function, only internal usage.
		 */
		void readOneRegister(QString &str,int &reg);

		/**
		 * Converts operandText to an integer, or throws a SIG_Exception that names lineText.
		 */
		int operandTextToInteger( const QString &operandText, const QString &lineText ) const;

		/**
		 * The sum of the weights of the instructions that languageParameters allows. Sets the hasCommand entry of each of them to true.
		 */
		long totalWeightOfAllowedInstructions( const SIGEL_Robot::SIG_LanguageParameters &languageParameters, const QList<int> &prob, QList<bool> &hasCommand ) const;

		/**
		 * The allowed instruction in whose range of weights randomValue is, or COPY if it is in none.
		 */
		RobotInstruction selectInstructionByWeight( long randomValue, const QList<bool> &hasCommand, const QList<int> &prob ) const;

		/**
		 *  The type of the current robot inbstruction, e.g. LOAD, COPY, ...
		 */
		RobotInstruction instructionType;

		/**
		 * This array contains the elements of a program line. Elements are coded with values
		 * between 0 and MAXINT. The meaning of these elements is dependend on the instruction
		 * type. Elements can represent registers or integer values.
		 */
		QList< int > element;
	};

}

#endif // SIGEL_PROGRAMPACKAGE_SIG_PROGRAMLINE_H
