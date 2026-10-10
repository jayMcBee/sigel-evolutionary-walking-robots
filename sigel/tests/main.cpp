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
#include "MT_GPSystem/MT_SearchTest.h"
#include "SIGEL_GP/SIG_GPIndividualTest.h"
#include "SIGEL_GP/SIG_GPPopulationTest.h"
#include "SIGEL_Robot/SIG_LanguageParametersTest.h"
#include "SIGEL_Robot/SIG_LinkTest.h"
#include "SIGEL_Robot/SIG_MaterialTest.h"
#include "SIGEL_Robot/SIG_RobotTest.h"
#include "SIGEL_Tools/SIG_RandomizerTest.h"

#include <QtTest>

#include <cstdio>
#include <memory>
#include <vector>

// Runs each test class. Returns 1 if a test function failed, else 0.
//
// There is no application object: with one, SIG_GPPopulation::addRandomIndividuals
// makes a progress dialog.
int main( int argc, char *argv[] ) {
  std::vector<std::unique_ptr<QObject>> testClasses;
  testClasses.push_back( std::make_unique<MT_SearchTest>() );
  testClasses.push_back( std::make_unique<SIGEL_GP::SIG_GPIndividualTest>() );
  testClasses.push_back( std::make_unique<SIGEL_GP::SIG_GPPopulationTest>() );
  testClasses.push_back( std::make_unique<SIGEL_Robot::SIG_LanguageParametersTest>() );
  testClasses.push_back( std::make_unique<SIGEL_Robot::SIG_LinkTest>() );
  testClasses.push_back( std::make_unique<SIGEL_Robot::SIG_MaterialTest>() );
  testClasses.push_back( std::make_unique<SIGEL_Robot::SIG_RobotTest>() );
  testClasses.push_back( std::make_unique<SIGEL_Tools::SIG_RandomizerTest>() );

  int failures = 0;
  for ( const std::unique_ptr<QObject> &testClass : testClasses )
    failures += QTest::qExec( testClass.get(), argc, argv );

  // Qt Test gives a total for each test class only.
  printf( "%d test classes, %d failures\n", static_cast<int>( testClasses.size() ), failures );

  return failures == 0 ? 0 : 1;
}
