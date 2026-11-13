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
#include "MT_GPSystem/MT_TrainingCase.h"

MT_TrainingCase::MT_TrainingCase(QTextStream &File)
{
	QString TCaseString("TrainingCases:" );
	QString PresentLine = File.readLine();

	while ((PresentLine != TCaseString) && !(File.atEnd()))
		PresentLine = File.readLine();

	if ((PresentLine == TCaseString) && !(File.atEnd()))
	{
		Name = (File.readLine()).toInt();
		Fitness = (File.readLine()).toDouble();
		Boundary = (File.readLine()).toInt();

		TranslateIndividual = new MT_TranslatedIndividual(File);	
	}
}

MT_TrainingCase::MT_TrainingCase()
{
	Name = -1;
	Fitness = -1.0;
	Boundary = 0;
}

MT_TrainingCase::~MT_TrainingCase()
{
	delete TranslateIndividual;
}

MT_TrainingCase::MT_TrainingCase(double Fit, MT_TranslatedIndividual *TransIndi, int Na, int boun)
{
	Name = Na;
	Fitness = Fit;
	TranslateIndividual = TransIndi;
	Boundary = boun;
}

void MT_TrainingCase::writeToFileTCase(QTextStream &File)
{
	File << ("TrainingCases:\n");
	File << Name << Qt::endl;
	File << Fitness << Qt::endl;
	File << Boundary << Qt::endl << Qt::endl;

	TranslateIndividual->writeToFileTransIndi(File);
}

double MT_TrainingCase::getFitness()
{
	return Fitness; 
}

void MT_TrainingCase::setBoundary(int NewBoundary)
{
	Boundary = NewBoundary;
}

void MT_TrainingCase::setName(int NewName)
{
	Name = NewName;
}

MT_TranslatedIndividual * MT_TrainingCase::getIndividual()
{
	return TranslateIndividual;
}

int MT_TrainingCase::getName()
{
	return Name;
}

void MT_TrainingCase::setFitness(double Fit)
{
	Fitness=Fit;
}
