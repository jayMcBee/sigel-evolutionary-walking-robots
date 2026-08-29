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
  simulationParameter(simulationParameter)
{
  miscParam.fitnessName=fitnessName;
  miscParam.visualize=visualize;
  miscParam.actGeneration=0;
  miscParam.resetEveryGeneration=0;
};

SIGEL_GP::SIG_GPPVMData::~SIG_GPPVMData() {};


void SIGEL_GP::SIG_GPPVMData::sendQStringToPVM(QString str, int taskId, int messageId)
{
  // finalLength sizes the receiver's buffer, so it must count the BYTES that
  // go on the wire, not the characters. It was str.length() + 1 here and in
  // the 2003 original, which is the same number only for ASCII: 20 'u'-umlauts
  // are 20 characters and 40 UTF-8 bytes, and getQStringFromPVM() below then
  // let pvm_upkstr write 41 bytes into a 21-byte buffer. Confirmed as a
  // heap-buffer-overflow under AddressSanitizer -- PORTING.md Phase P.
  // NOTE THE + 2, which is not a typo. Q2CString::size() reported
  // QByteArray::size() + 1, because Qt 2's QCString counted the terminating
  // NUL in its length. So "size() + 1" here was byte length + TWO: one byte
  // for the NUL that pvm_upkstr writes, and one spare. QByteArray::size() is
  // the plain byte count, so the same wire value needs + 2.
  //
  // Preserved rather than tightened. The receiver below sizes its buffer from
  // this number and pvm_upkstr writes byte length + 1, so + 1 would fit
  // exactly and + 2 leaves one byte of margin. Dropping that margin is a
  // behaviour change on the wire, and not this step's to make.
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
  QString miscString = cutAfterFiveHashes( file );

  QTextStream simParStream( &simParString, QIODeviceBase::ReadOnly );
  QTextStream environmentStream( &environmentString, QIODeviceBase::ReadOnly );
  QTextStream programStream( &programString, QIODeviceBase::ReadOnly );
  QTextStream robotStream( &robotString, QIODeviceBase::ReadOnly );
  QTextStream miscStream( &miscString, QIODeviceBase::ReadOnly );

  simulationParameter.readFromFile( simParStream );
  environment.readFromFile( environmentStream );
  program.readFromFile( programStream );
  robot.readFromFileTransfer( robotStream );

  QString bufferString;
  bufferString = miscStream.readLine(); // FITNESSNAME
  bufferString = miscStream.readLine();
  miscParam.fitnessName = bufferString;
  bufferString = miscStream.readLine(); // ACTGENERATION
  bufferString = miscStream.readLine();
  miscParam.actGeneration = bufferString.toInt();
  bufferString = miscStream.readLine(); // RESETEVERYGENERATION
  bufferString = miscStream.readLine();
  miscParam.resetEveryGeneration = bufferString.toInt();
  bufferString = miscStream.readLine(); //VISUALIZE
  bufferString = miscStream.readLine();
  int visuInt = bufferString.toInt();
  if (visuInt == 1)
    miscParam.visualize=true;
  else
    miscParam.visualize=false;
};

void SIGEL_GP::SIG_GPPVMData::savePVMDataTransfer(QTextStream & file,
						  SIGEL_Program::SIG_Program const &program)
{
  file.setRealNumberPrecision( 50 );

  SIGEL_Program::SIG_Program &usedProgram = const_cast< SIGEL_Program::SIG_Program& >(program);

  QString fiveHashesLine("\n#####\n");
  simulationParameter.writeToFile(file);
  file << fiveHashesLine;
  environment.writeToFile(file);
  file << fiveHashesLine;
  usedProgram.writeToFile(file);
  file << fiveHashesLine;
  robot.writeToFileTransfer(file);
  file << fiveHashesLine;
  file << "FITNESSNAME\n";
  file << miscParam.fitnessName << "\n";
  file << "ACTUALGENERATION\n";
  file << miscParam.actGeneration << "\n";
  file << "RESETEVERYGENERATION\n";
  file << miscParam.resetEveryGeneration << "\n";
  file << "VISUALIZE\n";
  if (miscParam.visualize)
    file << 1 << "\n";
  else
    file << 0 << "\n"; 
  file << fiveHashesLine;
};

void SIGEL_GP::SIG_GPPVMData::setVisualize(bool visu)
{
  miscParam.visualize=visu;
};

bool SIGEL_GP::SIG_GPPVMData::getVisualize()
{
  return miscParam.visualize;
};

QString SIGEL_GP::SIG_GPPVMData::getFitnessFunctionName()
{
  return miscParam.fitnessName;
};

void SIGEL_GP::SIG_GPPVMData::setFitnessFunctionName( QString name )
{
  miscParam.fitnessName = name;
};
