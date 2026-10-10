/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler
  Copyright 2026 Jan Barnholt (SIGEL 2.0)

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
#include "SIGEL_Robot/SIG_LanguageParametersTest.h"

#include "SIGEL_Robot/SIG_CommandParameters.h"
#include "SIGEL_Robot/SIG_LanguageParameters.h"
#include "SIGEL_Robot/SIG_RobotExceptions.h"

#include <QtTest>

void SIGEL_Robot::SIG_LanguageParametersTest::defaultConstructorGivesTheStandardLanguage()
{
  SIG_LanguageParameters languageParameters;

  QCOMPARE( languageParameters.getRegisterWidth(), 8 );
  QCOMPARE( languageParameters.getMemorySize(), 8 );
  QCOMPARE( languageParameters.getMaximalDelayTime(), 5000 );

  // The order is not alphabetical, and it goes into each saved experiment.
  const QStringList names = { "MUL", "MOVE", "CMP", "COPY", "LOAD", "JMP", "SENSE", "NOP",
                              "SUB", "DIV", "MIN", "DELAY", "ADD", "MOD", "MAX" };
  QCOMPARE( languageParameters.getCommands().count(), names.count() );
  for ( int i = 0; i < names.count(); i++ )
    {
      QCOMPARE( languageParameters.getCommands().at( i ).name, names.at( i ) );
      QCOMPARE( languageParameters.getCommands().at( i ).value->getDuration(), 0.001 );
    }
}

void SIGEL_Robot::SIG_LanguageParametersTest::settersChangeTheValues()
{
  SIG_LanguageParameters languageParameters;

  languageParameters.setRegisterWidth( 12 );
  languageParameters.setMemorySize( 20 );
  languageParameters.setMaximalDelayTime( 300 );

  QCOMPARE( languageParameters.getRegisterWidth(), 12 );
  QCOMPARE( languageParameters.getMemorySize(), 20 );
  QCOMPARE( languageParameters.getMaximalDelayTime(), 300 );
}

void SIGEL_Robot::SIG_LanguageParametersTest::hasCommandFindsOnlyAKnownName()
{
  SIG_LanguageParameters languageParameters;

  QVERIFY( languageParameters.hasCommand( "MOVE" ) );
  QVERIFY( !languageParameters.hasCommand( "MISSING" ) );
}

void SIGEL_Robot::SIG_LanguageParametersTest::removeCommandRemovesTheCommand()
{
  SIG_LanguageParameters languageParameters;
  SIG_CommandParameters *command = new SIG_CommandParameters();
  languageParameters.addCommand( "CHECKED", command );
  QCOMPARE( languageParameters.getCommand( "CHECKED" ), command );

  // SIG_LanguageParameters owns its commands; removeCommand frees this one.
  languageParameters.removeCommand( "CHECKED" );

  QCOMPARE( languageParameters.getCommand( "CHECKED" ), nullptr );
}

void SIGEL_Robot::SIG_LanguageParametersTest::removeCommandLeavesTheOtherCommands()
{
  SIG_LanguageParameters languageParameters;

  languageParameters.removeCommand( "MOVE" );

  QCOMPARE( languageParameters.getCommands().count(), 14 );
  QVERIFY( !languageParameters.hasCommand( "MOVE" ) );
  QCOMPARE( languageParameters.getCommands().at( 0 ).name, QString( "MUL" ) );
  QCOMPARE( languageParameters.getCommands().at( 1 ).name, QString( "CMP" ) );
}

void SIGEL_Robot::SIG_LanguageParametersTest::removeCommandWithUnknownNameChangesNothing()
{
  SIG_LanguageParameters languageParameters;

  languageParameters.removeCommand( "MISSING" );

  QCOMPARE( languageParameters.getCommands().count(), 15 );
}

void SIGEL_Robot::SIG_LanguageParametersTest::writeToFileTransferWritesTheCommandsInOrder()
{
  SIG_LanguageParameters languageParameters;
  languageParameters.setRegisterWidth( 12 );
  languageParameters.setMemorySize( 20 );
  languageParameters.setMaximalDelayTime( 300 );
  languageParameters.removeCommand( "MOVE" );
  languageParameters.getCommand( "CMP" )->setDuration( 0.5 );
  QString written;
  QTextStream stream( &written );

  languageParameters.writeToFileTransfer( stream );

  QCOMPARE( written, QString( "LanguageParameters 12 20 300 14\n"
                              "MUL CommandParameters 0.001\n"
                              "CMP CommandParameters 0.5\n"
                              "COPY CommandParameters 0.001\n"
                              "LOAD CommandParameters 0.001\n"
                              "JMP CommandParameters 0.001\n"
                              "SENSE CommandParameters 0.001\n"
                              "NOP CommandParameters 0.001\n"
                              "SUB CommandParameters 0.001\n"
                              "DIV CommandParameters 0.001\n"
                              "MIN CommandParameters 0.001\n"
                              "DELAY CommandParameters 0.001\n"
                              "ADD CommandParameters 0.001\n"
                              "MOD CommandParameters 0.001\n"
                              "MAX CommandParameters 0.001\n" ) );
}

void SIGEL_Robot::SIG_LanguageParametersTest::writtenTextIsReadBack()
{
  SIG_LanguageParameters original;
  original.setRegisterWidth( 12 );
  original.setMemorySize( 20 );
  original.setMaximalDelayTime( 300 );
  original.removeCommand( "MOVE" );
  original.getCommand( "CMP" )->setDuration( 0.5 );
  QString written;
  QTextStream writeStream( &written );
  original.writeToFileTransfer( writeStream );

  // The written text starts with the token, so the token is read here.
  QTextStream readStream( &written, QIODevice::ReadOnly );
  SIG_LanguageParameters copy( readStream, true );

  QCOMPARE( copy.getRegisterWidth(), 12 );
  QCOMPARE( copy.getMemorySize(), 20 );
  QCOMPARE( copy.getMaximalDelayTime(), 300 );
  QCOMPARE( copy.getCommands().count(), original.getCommands().count() );
  for ( int i = 0; i < copy.getCommands().count(); i++ )
    {
      QCOMPARE( copy.getCommands().at( i ).name, original.getCommands().at( i ).name );
      QCOMPARE( copy.getCommands().at( i ).value->getDuration(), original.getCommands().at( i ).value->getDuration() );
    }
}

// SIG_Robot reads the token itself and then gives the stream to this class.
void SIGEL_Robot::SIG_LanguageParametersTest::streamInARobotHasNoToken()
{
  QString text = "4 6 100 2\nADD CommandParameters 0.5\nSUB CommandParameters 0.25\n";
  QTextStream stream( &text, QIODevice::ReadOnly );

  SIG_LanguageParameters languageParameters( stream );

  QCOMPARE( languageParameters.getRegisterWidth(), 4 );
  QCOMPARE( languageParameters.getMemorySize(), 6 );
  QCOMPARE( languageParameters.getMaximalDelayTime(), 100 );
  QCOMPARE( languageParameters.getCommands().count(), 2 );
  QCOMPARE( languageParameters.getCommands().at( 0 ).name, QString( "ADD" ) );
  QCOMPARE( languageParameters.getCommands().at( 0 ).value->getDuration(), 0.5 );
  QCOMPARE( languageParameters.getCommands().at( 1 ).name, QString( "SUB" ) );
  QCOMPARE( languageParameters.getCommands().at( 1 ).value->getDuration(), 0.25 );
}

void SIGEL_Robot::SIG_LanguageParametersTest::wrongTokenThrows()
{
  QString text = "Environment 8 8 5000 0\n";
  QTextStream stream( &text, QIODevice::ReadOnly );

  QVERIFY_THROWS_EXCEPTION( SIG_UnstreamingError, SIG_LanguageParameters languageParameters( stream, true ) );
}

void SIGEL_Robot::SIG_LanguageParametersTest::registerWidthOutsideOneToSixteenThrows()
{
  QString tooSmall = "0 8 5000 0\n";
  QTextStream tooSmallStream( &tooSmall, QIODevice::ReadOnly );
  QVERIFY_THROWS_EXCEPTION( SIG_UnstreamingError, SIG_LanguageParameters languageParameters( tooSmallStream ) );

  QString tooLarge = "17 8 5000 0\n";
  QTextStream tooLargeStream( &tooLarge, QIODevice::ReadOnly );
  QVERIFY_THROWS_EXCEPTION( SIG_UnstreamingError, SIG_LanguageParameters languageParameters( tooLargeStream ) );
}

void SIGEL_Robot::SIG_LanguageParametersTest::registerWidthOneAndSixteenAreAccepted()
{
  QString smallest = "1 8 5000 0\n";
  QTextStream smallestStream( &smallest, QIODevice::ReadOnly );
  SIG_LanguageParameters withSmallest( smallestStream );
  QCOMPARE( withSmallest.getRegisterWidth(), 1 );

  QString largest = "16 8 5000 0\n";
  QTextStream largestStream( &largest, QIODevice::ReadOnly );
  SIG_LanguageParameters withLargest( largestStream );
  QCOMPARE( withLargest.getRegisterWidth(), 16 );
}
