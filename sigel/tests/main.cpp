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
#include "MT_GPSystem/TST_MT_Search.h"
#include "SIGEL_Environment/TST_SIG_Terrain.h"
#include "SIGEL_GP/TST_SIG_GPExperiment.h"
#include "SIGEL_GP/TST_SIG_GPIndividual.h"
#include "SIGEL_GP/TST_SIG_GPOperations.h"
#include "SIGEL_GP/TST_SIG_GPParameter.h"
#include "SIGEL_GP/TST_SIG_GPPopulation.h"
#include "SIGEL_GP/TST_SIG_GPPVMData.h"
#include "SIGEL_GP/TST_SIG_GPPVMHost.h"
#include "SIGEL_Program/TST_SIG_Program.h"
#include "SIGEL_Program/TST_SIG_ProgramLine.h"
#include "SIGEL_Robot/TST_SIG_Body.h"
#include "SIGEL_Robot/TST_SIG_Drive.h"
#include "SIGEL_Robot/TST_SIG_LanguageParameters.h"
#include "SIGEL_Robot/TST_SIG_Link.h"
#include "SIGEL_Robot/TST_SIG_Material.h"
#include "SIGEL_Robot/TST_SIG_Robot.h"
#include "SIGEL_Robot/TST_SIG_VRMLGeometryReader.h"
#include "SIGEL_Simulation/TST_SIG_DynaMechsSimulationData.h"
#include "SIGEL_Simulation/TST_SIG_Interpreter.h"
#include "SIGEL_Simulation/TST_SIG_Register.h"
#include "SIGEL_Tools/TST_SIG_FileUtils.h"
#include "SIGEL_Tools/TST_SIG_Randomizer.h"

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
  testClasses.push_back( std::make_unique<TST_MT_Search>() );
  testClasses.push_back( std::make_unique<SIGEL_Environment::TST_SIG_Terrain>() );
  testClasses.push_back( std::make_unique<SIGEL_GP::TST_SIG_GPExperiment>() );
  testClasses.push_back( std::make_unique<SIGEL_GP::TST_SIG_GPIndividual>() );
  testClasses.push_back( std::make_unique<SIGEL_GP::TST_SIG_GPOperations>() );
  testClasses.push_back( std::make_unique<SIGEL_GP::TST_SIG_GPParameter>() );
  testClasses.push_back( std::make_unique<SIGEL_GP::TST_SIG_GPPopulation>() );
  testClasses.push_back( std::make_unique<SIGEL_GP::TST_SIG_GPPVMData>() );
  testClasses.push_back( std::make_unique<SIGEL_GP::TST_SIG_GPPVMHost>() );
  testClasses.push_back( std::make_unique<SIGEL_Program::TST_SIG_Program>() );
  testClasses.push_back( std::make_unique<SIGEL_Program::TST_SIG_ProgramLine>() );
  testClasses.push_back( std::make_unique<SIGEL_Robot::TST_SIG_Body>() );
  testClasses.push_back( std::make_unique<SIGEL_Robot::TST_SIG_Drive>() );
  testClasses.push_back( std::make_unique<SIGEL_Robot::TST_SIG_LanguageParameters>() );
  testClasses.push_back( std::make_unique<SIGEL_Robot::TST_SIG_Link>() );
  testClasses.push_back( std::make_unique<SIGEL_Robot::TST_SIG_Material>() );
  testClasses.push_back( std::make_unique<SIGEL_Robot::TST_SIG_Robot>() );
  testClasses.push_back( std::make_unique<SIGEL_Robot::TST_SIG_VRMLGeometryReader>() );
  testClasses.push_back( std::make_unique<SIGEL_Simulation::TST_SIG_DynaMechsSimulationData>() );
  testClasses.push_back( std::make_unique<SIGEL_Simulation::TST_SIG_Interpreter>() );
  testClasses.push_back( std::make_unique<SIGEL_Simulation::TST_SIG_Register>() );
  testClasses.push_back( std::make_unique<SIGEL_Tools::TST_SIG_FileUtils>() );
  testClasses.push_back( std::make_unique<SIGEL_Tools::TST_SIG_Randomizer>() );

  int failures = 0;
  for ( const std::unique_ptr<QObject> &testClass : testClasses )
    failures += QTest::qExec( testClass.get(), argc, argv );

  // Qt Test gives a total for each test class only.
  printf( "%d test classes, %d failures\n", static_cast<int>( testClasses.size() ), failures );

  return failures == 0 ? 0 : 1;
}
