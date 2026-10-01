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
#include <QByteArray>
#include <QList>
#include "SIGEL_GP/SIG_GPPVMData.h"

#include <qtextstream.h>
#include <pvm3.h>

#include "SIGEL_Tools/SIG_IO.h"

SIGEL_GP::SIG_GPPVMData::SIG_GPPVMData(SIGEL_Robot::SIG_Robot& robot,
       SIGEL_Environment::SIG_Environment& environment,
       SIGEL_Simulation::SIG_SimulationParameters& simulationParameter,
       QString fitnessName,
       bool visualize)
  : robot(robot),
  environment(environment),
  simulationParameter(simulationParameter),
  fitnessName(fitnessName),
  visualize(visualize),
  actGeneration(0),
  resetEveryGeneration(0),
  experimentName(),
  individualName(),
  individualFitness(0)
{
};

void SIGEL_GP::SIG_GPPVMData::sendQStringToPVM(QString str, int taskId, int messageId)
{
  // Bytes, not characters: pvm_upkstr writes into a buffer of this size.
  // The + 2 is not a typo: one byte for the NUL it writes, and one spare.
  // A null string must be sent as empty -- pvm_pkstr does strlen unguarded.
  const QByteArray qCStringBuffer = str.toUtf8();
  int finalLength = qCStringBuffer.size() + 2;

  pvm_initsend(PvmDataDefault);
  pvm_pkint(&finalLength,1,1);

  char const *cStringBuffer = qCStringBuffer.constData();
  pvm_pkstr( const_cast<char*>( cStringBuffer ) );
  pvm_send(taskId, messageId);
};

QString SIGEL_GP::SIG_GPPVMData::getQStringFromPVM(int taskId, int messageId) {
  int length;

  pvm_recv(taskId,messageId);
  pvm_upkint(&length,1,1);

  QList< char > buffer( length );

  pvm_upkstr( buffer.data() );

  QString result( buffer.data() );

  return result;
};

QString SIGEL_GP::SIG_GPPVMData::cutAfterFiveHashes(QTextStream& source)
{
  QString fiveHashes( "#####" );
  QString bufferString;
  QString resultString;

  while ( (bufferString = source.readLine()) != fiveHashes )
    resultString += ( bufferString + "\n" );

  return resultString;
};

void SIGEL_GP::SIG_GPPVMData::loadPVMDataTransfer(QTextStream & file,
        SIGEL_Program::SIG_Program & program)
{
  file.setRealNumberPrecision( 50 );

  QString simParString = cutAfterFiveHashes( file );
  QString environmentString = cutAfterFiveHashes( file );
  QString programString = cutAfterFiveHashes( file );
  QString robotString = cutAfterFiveHashes( file );
  QString settingsString = cutAfterFiveHashes( file );

  QTextStream simParStream( &simParString, QIODeviceBase::ReadOnly );
  QTextStream environmentStream( &environmentString, QIODeviceBase::ReadOnly );
  QTextStream programStream( &programString, QIODeviceBase::ReadOnly );
  QTextStream robotStream( &robotString, QIODeviceBase::ReadOnly );
  QTextStream settingsStream( &settingsString, QIODeviceBase::ReadOnly );

  simulationParameter.readFromFile( simParStream );
  environment.readFromFile( environmentStream );
  program.readFromFile( programStream );
  robot.readFromFileTransfer( robotStream );

  QString bufferString;
  bufferString = settingsStream.readLine(); // FITNESSNAME
  bufferString = settingsStream.readLine();
  fitnessName = bufferString;
  bufferString = settingsStream.readLine(); // ACTUALGENERATION
  bufferString = settingsStream.readLine();
  actGeneration = bufferString.toInt();
  bufferString = settingsStream.readLine(); // RESETEVERYGENERATION
  bufferString = settingsStream.readLine();
  resetEveryGeneration = bufferString.toInt();
  bufferString = settingsStream.readLine(); //VISUALIZE
  bufferString = settingsStream.readLine();
  int visuInt = bufferString.toInt();
  if (visuInt == 1)
    visualize=true;
  else
    visualize=false;
  bufferString = settingsStream.readLine(); // EXPERIMENTNAME
  bufferString = settingsStream.readLine();
  experimentName = bufferString;
  bufferString = settingsStream.readLine(); // INDIVIDUALNAME
  bufferString = settingsStream.readLine();
  individualName = bufferString;
  bufferString = settingsStream.readLine(); // INDIVIDUALFITNESS
  bufferString = settingsStream.readLine();
  individualFitness = bufferString.toDouble();
};

void SIGEL_GP::SIG_GPPVMData::savePVMDataTransfer(QTextStream & file,
						  SIGEL_Program::SIG_Program const &program)
{
  file.setRealNumberPrecision( 50 );

  QString fiveHashesLine("\n#####\n");
  simulationParameter.writeToFile(file);
  file << fiveHashesLine;
  environment.writeToFile(file);
  file << fiveHashesLine;
  program.writeToFile(file);
  file << fiveHashesLine;
  robot.writeToFileTransfer(file);
  file << fiveHashesLine;
  file << "FITNESSNAME\n";
  file << fitnessName << "\n";
  file << "ACTUALGENERATION\n";
  file << actGeneration << "\n";
  file << "RESETEVERYGENERATION\n";
  file << resetEveryGeneration << "\n";
  file << "VISUALIZE\n";
  if (visualize)
    file << 1 << "\n";
  else
    file << 0 << "\n"; 
  file << "EXPERIMENTNAME\n";
  file << experimentName << "\n";
  file << "INDIVIDUALNAME\n";
  file << individualName << "\n";
  file << "INDIVIDUALFITNESS\n";
  file << individualFitness << "\n";
  file << fiveHashesLine;
};

void SIGEL_GP::SIG_GPPVMData::setVisualize(bool visu)
{
  visualize=visu;
};

bool SIGEL_GP::SIG_GPPVMData::getVisualize()
{
  return visualize;
};

QString SIGEL_GP::SIG_GPPVMData::getFitnessFunctionName()
{
  return fitnessName;
};

void SIGEL_GP::SIG_GPPVMData::setFitnessFunctionName( QString name )
{
  fitnessName = name;
};

QString SIGEL_GP::SIG_GPPVMData::getExperimentName()
{
  return experimentName;
};

void SIGEL_GP::SIG_GPPVMData::setExperimentName( QString name )
{
  experimentName = name;
};

QString SIGEL_GP::SIG_GPPVMData::getIndividualName()
{
  return individualName;
};

void SIGEL_GP::SIG_GPPVMData::setIndividualName( QString name )
{
  individualName = name;
};

double SIGEL_GP::SIG_GPPVMData::getIndividualFitness()
{
  return individualFitness;
};

void SIGEL_GP::SIG_GPPVMData::setIndividualFitness( double fitness )
{
  individualFitness = fitness;
};
