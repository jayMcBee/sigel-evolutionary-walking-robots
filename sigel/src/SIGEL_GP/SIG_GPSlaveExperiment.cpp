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
#include "SIGEL_GP/SIG_GPSlaveExperiment.h"

#include <qfile.h>
#include <qtextstream.h>

#include "SIGEL_Tools/SIG_IO.h"
#include "SIGEL_Tools/SIG_FileUtils.h"

#include <pvm3.h>

SIGEL_GP::SIG_GPSlaveExperiment::SIG_GPSlaveExperiment(QString exp)
{
	autosavePath = "new";
}

SIGEL_GP::SIG_GPSlaveExperiment::SIG_GPSlaveExperiment()
{
  autosavePath = "new";
}

SIGEL_GP::SIG_GPSlaveExperiment::~SIG_GPSlaveExperiment()
{
  // This class owns its history entries.
  qDeleteAll( experimentHistory );
  experimentHistory.clear();
}

void SIGEL_GP::SIG_GPSlaveExperiment::loadExperiment(QTextStream & file)
{
  loadBlocks( readBlock( file ), file );
}

QString SIGEL_GP::SIG_GPSlaveExperiment::readBlock( QTextStream &file )
{
  QString const separator( "#####" );
  QString const missingSeparator( "The file ends before a '#####' separator. It is not a complete experiment file." );

  return SIGEL_Tools::SIG_FileUtils::readLinesUpTo( file, separator, missingSeparator );
}

void SIGEL_GP::SIG_GPSlaveExperiment::loadBlocks( QString simulationParameterBlock, QTextStream &file )
{
  QString environmentString = readBlock( file );
  QString gpParameterString = readBlock( file );
  QString populationString = readBlock( file );
  QString robotString = readBlock( file );
  QString experimentHistoryString = readBlock( file );

  QTextStream simParStream( &simulationParameterBlock, QIODeviceBase::ReadOnly );
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
}

void SIGEL_GP::SIG_GPSlaveExperiment::saveExperiment(QTextStream & file)
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
}

void SIGEL_GP::SIG_GPSlaveExperiment::writeHistoryToFileTransfer( QTextStream &file )
{
  for (const SIG_GPExperimentHistoryEntry *actEntry : experimentHistory)
    file << actEntry->print();
}

void SIGEL_GP::SIG_GPSlaveExperiment::readHistoryFromFileTransfer( QTextStream &file )
{
  qDeleteAll( experimentHistory );
  experimentHistory.clear();

  QString buffer = file.readLine();

  while (!buffer.isEmpty())
    {
      SIG_GPExperimentHistoryEntry *newEntry = new SIG_GPExperimentHistoryEntry( buffer );

      experimentHistory.append( newEntry );

      buffer = file.readLine();
    }
}

double SIGEL_GP::SIG_GPSlaveExperiment::calculateFitness(SIGEL_Program::SIG_Program & program)
{
 return 0;
}

SIGEL_GP::SIG_GPParameter& SIGEL_GP::SIG_GPSlaveExperiment::getGPParameter()
{
  return gpParameter;
}

SIGEL_GP::SIG_GPPopulation& SIGEL_GP::SIG_GPSlaveExperiment::getPopulation()
{
  return population;
}

SIGEL_GP::SIG_GPPopulation *SIGEL_GP::SIG_GPSlaveExperiment::getPopulationPointer()
{
  return &population;
}

void SIGEL_GP::SIG_GPSlaveExperiment::exportExperimentHistoryToGNUPlot( QString fileName )
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
}

QString SIGEL_GP::SIG_GPSlaveExperiment::getPath() {
	return autosavePath;
}

void SIGEL_GP::SIG_GPSlaveExperiment::setPath(QString path) {
	autosavePath = path;
}
