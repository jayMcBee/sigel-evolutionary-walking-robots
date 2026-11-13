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
#ifndef MT_GPSYSTEM_MT_INDIVIDUAL_H
#define MT_GPSYSTEM_MT_INDIVIDUAL_H

#include <QString>
#include <QTextStream>
#include <qstring.h>
#include <qtextstream.h>
#include "MT_GPSystem/MT_Program.h"
#include "MT_GPSystem/MT_Randomizer.h"	
#include "MT_GPSystem/MT_ErrorCode.h"
#include "MT_GPSystem/MT_GenesisType.h"

/*This class represent a Meta Individual, which contain especially the program 
* for Interpreting  
*/

class MT_Individual  

{
public:
	
	QString printProgramLine(int index);
	
	/* change the max Numbers of Variable, which can use by the Interpreter
	* if a variable name > NewNum so it will set of (variable name) mod NewNum
	*@post: all variable names are between 0 and NewNum-1 of all program lines */
	void changeMaxNumVariable(int NewNum);

	/* this function determine whether the currently fitness estimate for this Individuals, 
	* if this done then the function returns true */
	bool toBeEvaluated(int TSet);

	// increase the Age; Age = Age+1; 
	void increaseAge();

	/* a lot of get and set function for handling the attributes */
	void setTrainingsSet (int TName);
	void setFitness(double NewFitness);
	void setPosition(int Pos);
	void setMaxProgLen(int NewLen);
	void setTypeOfGenesis(MT_GenesisType Type);
	void setFitnessOfParent (double FitOfParent);
	void setNewProgram(MT_Program * NewProg);
	void setNewAge(int NewAge);
	void setNewName(int NewName);

	MT_GenesisType getTypeOfGenesis();
	double getFitnessOfParent ();
	int getPosition();
	double getFitness();
	int getName();
	int getAge();
	MT_ErrorCode getLastError();
	MT_Programline * getProgramLine(int Line);
	MT_Program * getProgram();

	// create a new Program, the older program will delete	
	void createProgram(MT_Randomizer *Randi);

	MT_Individual(MT_Program *Prog, bool Copy);
	
	MT_Individual(QTextStream &File);
	MT_Individual(int Na, MT_Randomizer *Randi);
	void writeToFileIndi(QTextStream &File);
	MT_Individual();
	virtual ~MT_Individual();

private:

	MT_ErrorCode LastError;
	/* indicate the Fitness of a Parent; -1 = Parent not existing */
	double FitnessOfParent;

	/* indicate how the program were created; a mutated individual holds Mutation + X, X mutated elements */
	MT_GenesisType TypeOfGenesis;

	// the name of the Trainingset on which the Individual training last time 
	int TSetName;
	// indicate the location of the individual inside the population 
	int Position;
	// indicate the Age of the individual
	int Age;
	int Name;
	//the main part of the META individual, a pointer of the program 
	MT_Program * Program;
	// the fitness of the Progrom, semantic: greater fitness are better;     
	double Fitness;
};

#endif // MT_GPSYSTEM_MT_INDIVIDUAL_H
