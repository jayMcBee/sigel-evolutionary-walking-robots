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
#include "SIGEL_Environment/TST_SIG_Environment.h"

#include "SIGEL_Environment/SIG_Environment.h"

#include <QtTest>

void SIGEL_Environment::TST_SIG_Environment::loadTerrainGivesTheHeightsOfTheFloorFunction()
{
  SIG_Environment environment;
  environment.setFloorDimensionX( 4 );
  environment.setFloorDimensionZ( 4 );
  environment.setFloorFunction( "x+10*z" );

  // The function's x counts along the second index, its z along the first.
  QCOMPARE( loadedHeight( environment, 0, 0 ), 0.0 );
  QCOMPARE( loadedHeight( environment, 0, 3 ), 3.0 );
  QCOMPARE( loadedHeight( environment, 2, 1 ), 21.0 );
}

void SIGEL_Environment::TST_SIG_Environment::loadTerrainGivesTheDimensionsOfTheFloor()
{
  SIG_Environment environment;
  environment.setFloorDimensionX( 7 );
  environment.setFloorDimensionZ( 7 );

  dmEnvironment dynaMechsEnvironment;
  environment.loadTerrain( dynaMechsEnvironment );

  int firstDimension = 0;
  int secondDimension = 0;
  Float spacing = 0;
  dynaMechsEnvironment.getTerrainData( firstDimension, secondDimension, spacing );

  QCOMPARE( firstDimension, 7 );
  QCOMPARE( secondDimension, 7 );
  QCOMPARE( spacing, 1.0 );
}

void SIGEL_Environment::TST_SIG_Environment::anotherEnvironmentDoesNotChangeTheTerrain()
{
  SIG_Environment raised;
  raised.setFloorFunction( "2" );

  // A slave makes a default environment first; its flat floor must not reach the raised one.
  SIG_Environment flat;
  dmEnvironment flatDynaMechsEnvironment;
  flat.loadTerrain( flatDynaMechsEnvironment );

  QCOMPARE( loadedHeight( raised, 5, 5 ), 2.0 );
  QCOMPARE( loadedHeight( flat, 5, 5 ), 0.0 );
}

void SIGEL_Environment::TST_SIG_Environment::floorFunctionThatCannotBeParsedGivesAFlatFloor()
{
  SIG_Environment raised;
  raised.setFloorFunction( "2" );
  dmEnvironment raisedDynaMechsEnvironment;
  raised.loadTerrain( raisedDynaMechsEnvironment );

  SIG_Environment environment;
  environment.setFloorDimensionX( 4 );
  environment.setFloorDimensionZ( 4 );
  environment.setFloorFunction( "sin(x" );

  QCOMPARE( loadedHeight( environment, 3, 3 ), 0.0 );
}

void SIGEL_Environment::TST_SIG_Environment::pictureFileThatDoesNotExistGivesAFlatFloor()
{
  SIG_Environment environment;
  environment.setFloorDimensionX( 4 );
  environment.setFloorDimensionZ( 4 );
  environment.setFloorPictureFile( "/this/picture/does/not/exist.pgm" );
  environment.setFloorFuncSelected( false );

  QCOMPARE( loadedHeight( environment, 3, 3 ), 0.0 );
}

void SIGEL_Environment::TST_SIG_Environment::loadTerrainNeedsNoSigelRoot()
{
  const QByteArray sigelRoot = qgetenv( "SIGEL_ROOT" );
  const bool sigelRootWasSet = qEnvironmentVariableIsSet( "SIGEL_ROOT" );
  qunsetenv( "SIGEL_ROOT" );

  SIG_Environment environment;
  environment.setFloorFunction( "2" );
  const double height = loadedHeight( environment, 5, 5 );

  if (sigelRootWasSet)
    qputenv( "SIGEL_ROOT", sigelRoot );

  QCOMPARE( height, 2.0 );
}

double SIGEL_Environment::TST_SIG_Environment::loadedHeight( const SIG_Environment &environment, int first, int second ) const
{
  dmEnvironment dynaMechsEnvironment;
  environment.loadTerrain( dynaMechsEnvironment );

  int firstDimension = 0;
  int secondDimension = 0;
  Float spacing = 0;
  Float **heights = dynaMechsEnvironment.getTerrainData( firstDimension, secondDimension, spacing );

  return heights[ first ][ second ];
}
