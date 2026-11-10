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
#ifndef SIGEL_SIMULATION_TST_SIG_DYNAMECHSSIMULATIONQUERIES_H
#define SIGEL_SIMULATION_TST_SIG_DYNAMECHSSIMULATIONQUERIES_H

#include "SIGEL_Robot/SIG_Joint.h"
#include "SIGEL_Tools/SIG_Matrix.h"
#include "SIGEL_Tools/SIG_Vector.h"

#include <QList>
#include <QObject>
#include <QString>

namespace SIGEL_Robot
{

  class SIG_Geometry;
  class SIG_Link;
  class SIG_Robot;

}

namespace SIGEL_Simulation
{

  class SIG_DynaMechsSimulationQueries;

  /**
   * The unit tests of SIG_DynaMechsSimulationQueries.
   */
  class TST_SIG_DynaMechsSimulationQueries : public QObject
  {
    Q_OBJECT

  private slots:

    void getCurrentSimulationSecondsIsTheFrameTimesTheStepSize_data();
    void getCurrentSimulationSecondsIsTheFrameTimesTheStepSize();

    void getCurrentSimulationWholeSecondsCutsTheTimeToWholeSeconds_data();
    void getCurrentSimulationWholeSecondsCutsTheTimeToWholeSeconds();

    void getLinkPositionGivesTheStartPositionForARobotWithoutJoints();

    void getLinkPositionAndOrientationPutEachLinkWhereTheSimulationHasIt_data();
    void getLinkPositionAndOrientationPutEachLinkWhereTheSimulationHasIt();

    void getLinkPositionFollowsTheRobotWhenItFalls();

    void getLinkOrientationGivesNoTurnForARobotWithoutJoints();

    void getLinkOrientationFollowsTheRobotWhenItTurns();

    void getRootNumberGivesTheNumberOfTheRootLink();

    void getLinkCountGivesTheNumberOfLinksOfTheRobot();

    void getUsedForcesGivesOneForceForEachLink();

    void getUsedForcesGivesZeroForARobotThatNothingPushes();

    void getUsedForcesGivesTheMomentOfAJointOnTheLinkItTurns();

    void getUsedForcesGivesTheForceOfASliderOnTheLinkItMoves();

    void senseReadsTheJointPositionInTheRangeOfTheRegister_data();
    void senseReadsTheJointPositionInTheRangeOfTheRegister();

    void senseWritesTheReadingIntoTheFirstRegisterOnly();

    void senseReadsTheJointPositionAfterTheJointHasMoved();

    void senseReadsAJointWithTheRootOnTheRight_data();
    void senseReadsAJointWithTheRootOnTheRight();

    void senseReadsTheJointThatTheSensorIsOn_data();
    void senseReadsTheJointThatTheSensorIsOn();

    void senseReadsAJointPushedPastItsLimitAsTheEndOfTheRegister_data();
    void senseReadsAJointPushedPastItsLimitAsTheEndOfTheRegister();

    void senseReadsAJointWithoutLimitsByItsAngle();

    void senseLeavesTheRegisterForASliderWithoutLimits();

    void senseReadsContactWithTheGroundAsTheTopOfTheRegister_data();
    void senseReadsContactWithTheGroundAsTheTopOfTheRegister();

    void senseReadsNoContactAsTheBottomOfTheRegister_data();
    void senseReadsNoContactAsTheBottomOfTheRegister();

    void senseReadsTheContactOfALinkBehindAJoint_data();
    void senseReadsTheContactOfALinkBehindAJoint();

    void senseReadsContactOfALinkWithOnlyPartOfItOnTheGround_data();
    void senseReadsContactOfALinkWithOnlyPartOfItOnTheGround();

    void senseReadsTheContactOfALinkAsItLandsAndLiftsOff();

    void senseReadsALevelLinkAsTheMiddleOfTheRegister_data();
    void senseReadsALevelLinkAsTheMiddleOfTheRegister();

    void senseReadsATiltAboutOneLevelAxisOnlyAndPitchAndRollUseDifferentAxes();

    void senseReadsATiltOnTheScaleOfTheRegister();

    void senseReadsTheSameTiltWhenTheRobotTurnsAboutTheVertical_data();
    void senseReadsTheSameTiltWhenTheRobotTurnsAboutTheVertical();

    void senseReadsATiltPastAQuarterTurnAsAQuarterTurn();

    void senseReadsTheTiltThatTheSimulationHasMade_data();
    void senseReadsTheTiltThatTheSimulationHasMade();

    void senseReadsTheTiltOfALinkBehindAJoint_data();
    void senseReadsTheTiltOfALinkBehindAJoint();

    void senseReadsTheTiltOfALinkThatItsJointHasTilted_data();
    void senseReadsTheTiltOfALinkThatItsJointHasTilted();

    void senseTakesTheSensorsInTurnAsTheSensorNumberCounts_data();
    void senseTakesTheSensorsInTurnAsTheSensorNumberCounts();

    void senseReadsEachSensorOfARobotWithSensorsOfEveryType();

    void senseLeavesTheRegistersAloneWhenTheRobotHasNoSensor();

  private:

    static void addFace( SIGEL_Robot::SIG_Geometry &geometry, const QList< SIG_Vector > &corners );
    static SIGEL_Robot::SIG_Link *addBoxLink( SIGEL_Robot::SIG_Robot &robot, const QString &name, SIG_Vector low, SIG_Vector high );
    static SIGEL_Robot::SIG_Joint *addJoint( SIGEL_Robot::SIG_Robot &robot, SIGEL_Robot::SIG_Joint::JointType type, int number,
      SIGEL_Robot::SIG_Link *left, SIGEL_Robot::SIG_Link *right, double minimum, double maximum, double start );
    static SIGEL_Robot::SIG_Joint *addBaseAndArm( SIGEL_Robot::SIG_Robot &robot, SIGEL_Robot::SIG_Joint::JointType type,
      double minimum, double maximum, double start );

    /** The value that sense writes into the first of four registers of this size. */
    static int reading( const SIG_DynaMechsSimulationQueries &queries, int sensorNumber, int registerSize );

    /** The turn by degrees about the coordinate axis x ( 0 ), y ( 1 ) or z ( 2 ), by the right-hand rule. */
    static SIG_Matrix turnAbout( int axis, double degrees );

    /** The reading of a pitch or a roll sensor on a box that starts in this orientation. */
    static int tiltReading( bool pitch, SIG_Matrix orientation );

    /** The readings of the sensor numbers from the register's bottom up, for sensors of these kinds. */
    static QList< int > sensorReadings( const QList< int > &kinds );

    static double armAlongBaseY( const SIG_DynaMechsSimulationQueries &queries );

    static SIG_Matrix product( SIG_Matrix left, SIG_Matrix right );

    static SIG_Matrix transposed( SIG_Matrix matrix );

    static QString worldBoxMismatch( const SIGEL_Robot::SIG_Robot &robot, const SIG_DynaMechsSimulationQueries &queries, int number,
      const QList< double > &box );

    /** Empty if the two matrices agree to 1e-9, else the first entry where they differ. */
    static QString matrixMismatch( SIG_Matrix actual, SIG_Matrix expected );

    /** Empty if the two vectors agree to tolerance, else both of them. */
    static QString vectorMismatch( SIG_Vector actual, SIG_Vector expected, double tolerance );
  };

}

#endif // SIGEL_SIMULATION_TST_SIG_DYNAMECHSSIMULATIONQUERIES_H
