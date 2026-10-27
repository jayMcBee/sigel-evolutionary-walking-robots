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
#include "SIGEL_GP/SIG_GPExperiment.h"

#include <qtextstream.h>

#include "SIGEL_Tools/SIG_FileUtils.h"

SIGEL_GP::SIG_GPExperiment::SIG_GPExperiment(QString exp)
	: SIG_GPSlaveExperiment( exp )
{
	mtController = std::make_unique< MT_Controller >(*this);
}

SIGEL_GP::SIG_GPExperiment::SIG_GPExperiment()
{
  mtController = std::make_unique< MT_Controller >(*this);
}

void SIGEL_GP::SIG_GPExperiment::loadExperiment(QTextStream & file)
{
  QString const separator( "#####" );
  QString const missingSeparator( "The file ends before a '#####' separator. It is not a complete experiment file." );

  QString simParString;

  // check if there's a file for the meta-system to be loaded
  QString metaFileString = SIGEL_Tools::SIG_FileUtils::readLinesUpTo( file, separator, missingSeparator );

  QTextStream metaFileStream( &metaFileString, QIODeviceBase::ReadOnly );

  if(metaFileStream.readLine() == "MetaInUse"){
	QString tmpFileString = metaFileStream.readLine();
	if(int index = tmpFileString.indexOf("fileName=") != -1 ){
		mtController->readFromFile( tmpFileString.mid(index+8) );
	}

	simParString = SIGEL_Tools::SIG_FileUtils::readLinesUpTo( file, separator, missingSeparator );
  } else
	simParString = metaFileString;
  QString environmentString = SIGEL_Tools::SIG_FileUtils::readLinesUpTo( file, separator, missingSeparator );
  QString gpParameterString = SIGEL_Tools::SIG_FileUtils::readLinesUpTo( file, separator, missingSeparator );
  QString populationString = SIGEL_Tools::SIG_FileUtils::readLinesUpTo( file, separator, missingSeparator );
  QString robotString = SIGEL_Tools::SIG_FileUtils::readLinesUpTo( file, separator, missingSeparator );
  QString experimentHistoryString = SIGEL_Tools::SIG_FileUtils::readLinesUpTo( file, separator, missingSeparator );

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
}

void SIGEL_GP::SIG_GPExperiment::saveExperiment(QTextStream & file)
{
  QString fiveHashesLine("\n#####\n");

  if(mtController->IsEnabled()){
	  mtController->writeToFile(file, experimentName, environment.getAutosaveIntervalGenerations());
  }

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
