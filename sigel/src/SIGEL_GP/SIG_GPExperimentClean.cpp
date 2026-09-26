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
#include "SIGEL_GP/SIG_GPExperimentClean.h"

#include <qfile.h>
#include <qtextstream.h>

#include "SIGEL_Tools/SIG_IO.h"
#include "SIGEL_Tools/SIG_Exception.h"

#include <pvm3.h>

SIGEL_GP::SIG_GPExperiment::SIG_GPExperiment(QString exp)
{
	autosavePath = "new";
};

SIGEL_GP::SIG_GPExperiment::SIG_GPExperiment()
{
  autosavePath = "new";
};

SIGEL_GP::SIG_GPExperiment::~SIG_GPExperiment()
{
  // This class owns its history entries.
  qDeleteAll( experimentHistory );
  experimentHistory.clear();
};

QString SIGEL_GP::SIG_GPExperiment::cutAfterFiveHashes(QTextStream& source)
{
  QString fiveHashes( "#####" );
  QString bufferString;
  QString resultString;

  while ( (bufferString = source.readLine()) != fiveHashes ) {
    if ( bufferString.isNull() )   // the end of the file
      throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__, "The file ends before a '#####' separator. It is not a complete experiment file." );
    resultString += ( bufferString + "\n" );
  }

  return resultString;
};

void SIGEL_GP::SIG_GPExperiment::loadExperiment(QTextStream & file)
{
  QString simParString = cutAfterFiveHashes( file );
  QString environmentString = cutAfterFiveHashes( file );
  QString gpParameterString = cutAfterFiveHashes( file );
  QString populationString = cutAfterFiveHashes( file );
  QString robotString = cutAfterFiveHashes( file );
  QString experimentHistoryString = cutAfterFiveHashes( file );

  QTextStream simParStream( &simParString, QIODeviceBase::ReadOnly );
  QTextStream environmentStream( &environmentString, QIODeviceBase::ReadOnly );
  QTextStream gpParameterStream( &gpParameterString, QIODeviceBase::ReadOnly );
  QTextStream populationStream( &populationString, QIODeviceBase::ReadOnly );
  QTextStream robotStream( &robotString, QIODeviceBase::ReadOnly );
  QTextStream experimentHistoryStream( &experimentHistoryString, QIODeviceBase::ReadOnly );

  simulationParameter.readFromFile( simParStream );
  environment.readFromFile( environmentStream );
  gpParameter.readFromFile( gpParameterStream );
  population.readFromFile( populationStream );
  robot.readFromFileTransfer( robotStream );
  readHistoryFromFileTransfer( experimentHistoryStream );

  comment = file.readAll();
};

void SIGEL_GP::SIG_GPExperiment::saveExperiment(QTextStream & file)
{
  QString fiveHashesLine("\n#####\n");

  simulationParameter.writeToFile(file);
  file << fiveHashesLine;
  environment.writeToFile(file);
  file << fiveHashesLine;
  gpParameter.writeToFile(file);
  file << fiveHashesLine;
  population.savePool(file);
  file << fiveHashesLine;
  robot.writeToFileTransfer(file);
  file << fiveHashesLine;
  writeHistoryToFileTransfer(file);
  file << fiveHashesLine;

  file << comment;
};


void SIGEL_GP::SIG_GPExperiment::writeHistoryToFileTransfer( QTextStream &file )
{
  // Not first(): Qt 2's QList::first() returned null on an empty list, Qt 6's
  // is undefined there and still compiles.
  for (const SIG_GPExperimentHistoryEntry *actEntry : experimentHistory)
    file << actEntry->print();
};

void SIGEL_GP::SIG_GPExperiment::readHistoryFromFileTransfer( QTextStream &file )
{
  qDeleteAll( experimentHistory );
  experimentHistory.clear();

  QString buffer = file.readLine();

  while (!buffer.isEmpty())
    {
      SIG_GPExperimentHistoryEntry *newEntry = new SIG_GPExperimentHistoryEntry( buffer );

      experimentHistory.append( newEntry );

      buffer = file.readLine();
    };
};

double SIGEL_GP::SIG_GPExperiment::calculateFitness(SIGEL_Program::SIG_Program & program)
{
 return 0;
};

SIGEL_GP::SIG_GPParameter& SIGEL_GP::SIG_GPExperiment::getGPParameter()
{
  return gpParameter;
}

SIGEL_GP::SIG_GPPopulation& SIGEL_GP::SIG_GPExperiment::getPopulation()
{
  return population;
}

SIGEL_GP::SIG_GPPopulation *SIGEL_GP::SIG_GPExperiment::getPopulationPointer()
{
  return &population;
}

void SIGEL_GP::SIG_GPExperiment::exportExperimentHistoryToGNUPlot( QString fileName )
{
  QFile gnuPlotFile( fileName );

  if (gnuPlotFile.open( QIODeviceBase::WriteOnly ))
    {
      QTextStream gnuPlotStream( &gnuPlotFile );

      for (const SIG_GPExperimentHistoryEntry *actEntry : experimentHistory)
	gnuPlotStream << actEntry->getGenerationNo()
		      << " "
		      << actEntry->getMaxFitness()
		      << " "
		      << actEntry->getMinFitness()
		      << " "
		      << actEntry->getAverageFitness()
		      << "\n";

      gnuPlotFile.close();
  }
  else
    SIGEL_Tools::SIG_IO::cerr << "Could not export experiment history under "
			      << fileName
			      << "!" << Qt::endl;
};

QString SIGEL_GP::SIG_GPExperiment::getPath() {
	return autosavePath;
};

void SIGEL_GP::SIG_GPExperiment::setPath(QString _autosavePath) {
	autosavePath = _autosavePath;
};
