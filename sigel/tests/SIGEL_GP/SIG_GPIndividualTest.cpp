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
#include "SIGEL_GP/SIG_GPIndividualTest.h"

#include "SIGEL_GP/SIG_GPIndividual.h"
#include "SIGEL_GP/SIG_GPParameter.h"
#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_Robot/SIG_LanguageParameters.h"
#include "SIGEL_Tools/SIG_Exception.h"
#include "SIGEL_Tools/SIG_Randomizer.h"

#include <QtTest>

void SIGEL_GP::SIG_GPIndividualTest::writtenTextIsReadBack()
{
  std::unique_ptr< SIG_GPIndividual > original = randomIndividual();
  original->setName( "17" );
  original->setPoolPos( 4 );
  original->setFitness( 3.5 );
  original->setAge( 7 );
  QString written = writtenText( *original, false );

  SIG_GPIndividual copy;
  copy.readFromFile( written );

  QCOMPARE( copy.getName(), QString( "17" ) );
  QCOMPARE( copy.getPoolPos(), original->getPoolPos() );
  QCOMPARE( copy.getFitness(), 3.5 );
  QCOMPARE( copy.getAge(), original->getAge() );
  QVERIFY( original->getProgram().getProgramLength() >= 5 );
  QCOMPARE( copy.getProgram().getProgramLength(), original->getProgram().getProgramLength() );
  QCOMPARE( writtenText( copy, false ), written );
}

void SIGEL_GP::SIG_GPIndividualTest::writtenTextWithoutHistoryHasNoHistoryBlock()
{
  std::unique_ptr< SIG_GPIndividual > original = randomIndividual();

  QVERIFY( writtenText( *original, true ).contains( "HISTORY BEGIN{" ) );
  QVERIFY( !writtenText( *original, false ).contains( "HISTORY BEGIN{" ) );
}

void SIGEL_GP::SIG_GPIndividualTest::textWithoutHistoryBlockIsRead()
{
  std::unique_ptr< SIG_GPIndividual > original = randomIndividual();
  QString written = writtenText( *original, false );

  SIG_GPIndividual copy;
  copy.readFromFile( written );

  QVERIFY( copy.getHistory().isEmpty() );
  QCOMPARE( writtenText( copy, false ), written );
}

void SIGEL_GP::SIG_GPIndividualTest::historyIsReadBack()
{
  std::unique_ptr< SIG_GPIndividual > original = randomIndividual();
  original->addMutationInfo( "17", QDateTime( QDate( 2026, 1, 2 ), QTime( 3, 4, 5 ) ), 2 );
  QString written = writtenText( *original, true );

  SIG_GPIndividual copy;
  copy.readFromFile( written );

  QVERIFY( original->getHistory().size() >= 3 );
  QCOMPARE( copy.getHistory().join( "\n" ), original->getHistory().join( "\n" ) );
  QCOMPARE( writtenText( copy, true ), written );
}

// The history of an imported individual starts with a line break. The first load removes it.
void SIGEL_GP::SIG_GPIndividualTest::historyWithALineBreakAtTheStartIsTheSameAfterTheFirstLoad()
{
  SIG_GPIndividual original;
  original.addPreparationOfHistoryInfo();
  QVERIFY( original.getHistory().first().startsWith( "\n" ) );

  SIG_GPIndividual firstLoad;
  firstLoad.readFromFile( writtenText( original, true ) );
  QString afterFirstLoad = writtenText( firstLoad, true );

  SIG_GPIndividual secondLoad;
  secondLoad.readFromFile( afterFirstLoad );

  QVERIFY( afterFirstLoad.contains( "OLD INDIVIDUAL DATA DELETED" ) );
  QCOMPARE( writtenText( secondLoad, true ), afterFirstLoad );
}

// Older experiment files have the history on the line of HISTORY BEGIN{.
void SIGEL_GP::SIG_GPIndividualTest::historyWithoutLineBreakAtTheStartIsRead()
{
  std::unique_ptr< SIG_GPIndividual > original = randomIndividual();
  QString written = writtenText( *original, true );
  QVERIFY( written.contains( "HISTORY BEGIN{\n" ) );
  QString older = written;
  older.replace( "HISTORY BEGIN{\n", "HISTORY BEGIN{" );

  SIG_GPIndividual copy;
  copy.readFromFile( older );

  QCOMPARE( writtenText( copy, true ), written );
}

void SIGEL_GP::SIG_GPIndividualTest::textWithoutARequiredFieldIsRefused_data()
{
  QTest::addColumn< QString >( "field" );
  QTest::addColumn< QString >( "message" );

  QTest::newRow( "NAME" ) << "NAME='" << "An individual has no NAME field.";
  QTest::newRow( "POOLPOS" ) << "POOLPOS=" << "Individual '17' has no POOLPOS field.";
  QTest::newRow( "FITNESS" ) << "FITNESS=" << "Individual '17' has no FITNESS field.";
  QTest::newRow( "AGE" ) << "AGE=" << "Individual '17' has no AGE field.";
  QTest::newRow( "PROGRAM" ) << "PROGRAM BEGIN{" << "Individual '17' has no PROGRAM block.";
}

void SIGEL_GP::SIG_GPIndividualTest::textWithoutARequiredFieldIsRefused()
{
  QFETCH( QString, field );
  QFETCH( QString, message );
  std::unique_ptr< SIG_GPIndividual > original = randomIndividual();
  original->setName( "17" );
  QString broken = writtenText( *original, false );
  QVERIFY( broken.contains( field ) );
  broken.replace( field, "MISSING" );

  SIG_GPIndividual copy;
  QString thrown;
  try
    {
      copy.readFromFile( broken );
    }
  catch ( const SIGEL_Tools::SIG_Exception &e )
    {
      thrown = e.getMessage();
    }

  QVERIFY2( thrown.contains( message ), qPrintable( thrown ) );
}

std::unique_ptr< SIGEL_GP::SIG_GPIndividual > SIGEL_GP::SIG_GPIndividualTest::randomIndividual()
{
  SIGEL_Tools::SIG_Randomizer randomizer( 1 );
  SIG_GPParameter gpParameter;
  gpParameter.setMinIndLength( 5 );
  gpParameter.setMaxIndLength( 8 );
  SIGEL_Robot::SIG_LanguageParameters languageParameters;

  return std::make_unique< SIG_GPIndividual >( randomizer, gpParameter, languageParameters );
}

QString SIGEL_GP::SIG_GPIndividualTest::writtenText( SIG_GPIndividual &individual, bool withHistory )
{
  QString text;
  QTextStream stream( &text );
  individual.writeToFile( stream, withHistory );

  return text;
}
