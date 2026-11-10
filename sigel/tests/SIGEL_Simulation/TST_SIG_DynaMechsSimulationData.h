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
#ifndef SIGEL_SIMULATION_TST_SIG_DYNAMECHSSIMULATIONDATA_H
#define SIGEL_SIMULATION_TST_SIG_DYNAMECHSSIMULATIONDATA_H

#include "SIGEL_Robot/SIG_Drive.h"
#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Tools/SIG_Vector.h"

#include <QList>
#include <QObject>
#include <QString>

class dmEnvironment;

namespace SIGEL_Robot
{

  class SIG_Geometry;
  class SIG_Link;
  class SIG_Robot;

}

namespace SIGEL_Environment
{

  class SIG_Environment;

}

namespace SIGEL_Simulation
{

  class SIG_DynaMechsSimulationQueries;

  /**
   * The unit tests of SIG_DynaMechsSimulationData.
   */
  class TST_SIG_DynaMechsSimulationData : public QObject
  {
    Q_OBJECT

  private slots:

    void loadTerrainGivesTheHeightsOfTheFloorFunction();

    void loadTerrainGivesTheHeightsOfThePictureFile();

    void loadTerrainGivesTheDimensionsOfTheFloor();

    void eachLoadHasATerrainFileOfItsOwnAndLeavesNoneBehind();

    void loadTerrainNeedsNoSigelRoot();

    void loadTerrainGivesEachHeightOfASymmetricFloorFunction();

    void loadTerrainGivesTheDimensionsOfAFloorThatIsNotSquare();

    void constructorGivesDynaMechsTheGravityInItsAxes();

    void constructorGivesDynaMechsTheGroundConstants();

    void constructorPutsTheRootLinkAtTheStartPositionPlusTheInitialLocationOfTheRobot();

    void constructorRefusesASecondSimulationInTheSameProcess();

    void constructorMakesItsEnvironmentTheOneOfDynaMechsAndTheDestructorTakesItOut();

    void constructorTurnsTheRootLinkByTheInitialOrientationOfTheRobot_data();
    void constructorTurnsTheRootLinkByTheInitialOrientationOfTheRobot();

    void constructorTakesTheIntegratorOfTheSimulationParameters_data();
    void constructorTakesTheIntegratorOfTheSimulationParameters();

    void constructorGivesTheLinkOfAJointItsKindLimitsAndFriction_data();
    void constructorGivesTheLinkOfAJointItsKindLimitsAndFriction();

    void constructorGivesAJointWithEqualLimitsNoLimits();

    void constructorRefusesALinkThatTwoJointsReach();

    void constructorListsTheDrivesAndSensorsAtTheirNumbers();

    void constructorListsOrRefusesARelativeOrAbsoluteDrive_data();
    void constructorListsOrRefusesARelativeOrAbsoluteDrive();

    void simulationProgressSetsTheForceOfADriveToZeroWhenItsTimeAccountRunsOut_data();
    void simulationProgressSetsTheForceOfADriveToZeroWhenItsTimeAccountRunsOut();

    void simulationProgressMovesTheRobot();

    void constructorPutsBothLegsOfABodyWithTwoLegsWhereTheModelHasThem();

    void constructorPutsALinkWhereTheModelHasItWhenItsJointHasTheRootOnTheRight();

    void constructorPutsEachLinkOfAChainWhereTheModelHasIt();

    void constructorPutsTheLinksOfALegWithAHipAndAKneeOnCrossingAxesWhereTheModelHasThem();

    void destructorFreesTheLinks();

    void constructorGivesDynaMechsTheFloorOfTheEnvironment_data();
    void constructorGivesDynaMechsTheFloorOfTheEnvironment();

    void constructorPutsTheLinkBehindATranslationalJointWhereItsStartValuePutsIt();

    void constructorPutsTheLinkBehindARotationalJointWhereItsStartValuePutsIt();

    void simulationProgressStepsByTheStepSizeOfTheSimulationParameters();

  private:

    static void addFace( SIGEL_Robot::SIG_Geometry &geometry, const QList< SIG_Vector > &corners );
    static SIGEL_Robot::SIG_Link *addBoxLink( SIGEL_Robot::SIG_Robot &robot, const QString &name, SIG_Vector low, SIG_Vector high );
    static void buildBox( SIGEL_Robot::SIG_Robot &robot );
    static SIGEL_Robot::SIG_Joint *addJoint( SIGEL_Robot::SIG_Robot &robot, SIGEL_Robot::SIG_Joint::JointType type, int number,
      SIGEL_Robot::SIG_Link *base, SIGEL_Robot::SIG_Link *arm, double minimum, double maximum );
    static SIGEL_Robot::SIG_Joint *addBaseAndArm( SIGEL_Robot::SIG_Robot &robot, SIGEL_Robot::SIG_Joint::JointType type, double minimum, double maximum );
    static SIGEL_Robot::SIG_Drive *addDrive( SIGEL_Robot::SIG_Robot &robot, int number, SIGEL_Robot::SIG_Joint *joint, SIGEL_Robot::SIG_Drive::DriveMode mode );
    static void buildBaseAndArmWithADrive( SIGEL_Robot::SIG_Robot &robot, SIGEL_Robot::SIG_Drive::DriveMode mode );
    static void addHinge( SIGEL_Robot::SIG_Robot &robot, int number, SIGEL_Robot::SIG_Link *left, const SIG_Vector ( &leftPoints )[ 3 ],
      SIGEL_Robot::SIG_Link *right, const SIG_Vector ( &rightPoints )[ 3 ] );
    static void worldBox( const SIGEL_Robot::SIG_Robot &robot, SIGEL_Simulation::SIG_DynaMechsSimulationQueries &queries, int number,
      SIG_Vector &low, SIG_Vector &high );
    static QString worldBoxMismatch( const SIGEL_Robot::SIG_Robot &robot, SIGEL_Simulation::SIG_DynaMechsSimulationQueries &queries, int number,
      SIG_Vector low, SIG_Vector high );

    /** The height that DynaMechs holds at one grid point after loadTerrain. */
    double loadedHeight( const SIGEL_Environment::SIG_Environment &environment, int first, int second ) const;

    /** Fails the test unless the terrain has these dimensions and this height at each grid point. */
    void verifyFloorOfOneHeight( dmEnvironment &dynaMechsEnvironment, int firstDimension, int secondDimension, double height ) const;
  };

}

#endif // SIGEL_SIMULATION_TST_SIG_DYNAMECHSSIMULATIONDATA_H
