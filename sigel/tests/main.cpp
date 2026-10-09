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
#include "SIGEL_GP/SIG_GPPopulationTest.h"
#include "SIGEL_Robot/SIG_LanguageParametersTest.h"
#include "SIGEL_Robot/SIG_LinkTest.h"
#include "SIGEL_Robot/SIG_MaterialTest.h"
#include "SIGEL_Robot/SIG_RobotTest.h"
#include "SIGEL_Tools/SIG_RandomizerTest.h"

#include <QtTest>

// Runs each test class. Returns 1 if a test function failed, else 0.
//
// There is no application object: with one, SIG_GPPopulation::addRandomIndividuals
// makes a progress dialog.
int main( int argc, char *argv[] ) {
  int failures = 0;

  SIGEL_GP::SIG_GPPopulationTest populationTest;
  failures += QTest::qExec( &populationTest, argc, argv );

  SIGEL_Robot::SIG_LanguageParametersTest languageParametersTest;
  failures += QTest::qExec( &languageParametersTest, argc, argv );

  SIGEL_Robot::SIG_LinkTest linkTest;
  failures += QTest::qExec( &linkTest, argc, argv );

  SIGEL_Robot::SIG_MaterialTest materialTest;
  failures += QTest::qExec( &materialTest, argc, argv );

  SIGEL_Robot::SIG_RobotTest robotTest;
  failures += QTest::qExec( &robotTest, argc, argv );

  SIGEL_Tools::SIG_RandomizerTest randomizerTest;
  failures += QTest::qExec( &randomizerTest, argc, argv );

  return failures == 0 ? 0 : 1;
}
