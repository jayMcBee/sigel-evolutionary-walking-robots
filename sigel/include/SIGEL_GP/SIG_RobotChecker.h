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
#ifndef SIGEL_GP_SIG_ROBOTCHECKER_H
#define SIGEL_GP_SIG_ROBOTCHECKER_H

#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_GP/SIG_GPParameter.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"
#include "SIGEL_Simulation/SIG_StartPose.h"
#include "SIGEL_Tools/SIG_Vector.h"

#include <QList>
#include <qstring.h>
#include <qstringlist.h>

namespace SIGEL_GP
{

  /**
   * Reads a robot and its settings and lists what will probably make an
   * evolution fail or mislead. No finding stops a run.
   */
  class SIG_RobotChecker
  {

  public:

    // An error is a robot that cannot run at all.
    enum Kind { tError, tWarning, tSuggestion };

    /**
     * One finding. check is the number of the check that found it and
     * title its name; part the robot parts it is about; analysis what was
     * measured and what that does to a run; advice what to change.
     * value is the worst that was computed and limit what it was compared
     * with. Check 1 gives both in degrees, checks 2 and 7 as heights, check 4
     * in percent of the robot's size, check 5 in percent of the smaller link's
     * volume, check 6 in the joint's own unit, checks 8 and 9 in seconds.
     * Check 0 is a robot that cannot be simulated, check 10 a link without
     * mass. Check 11 is the robot left alone on the floor, in degrees of joint
     * motion.
     */
    struct Finding
    {
      int check;
      Kind kind;
      QString title;
      QString part;
      QString analysis;
      QString advice;
      double value;
      double limit;
    };

    SIG_RobotChecker( const SIGEL_Robot::SIG_Robot &robot,
                      const SIGEL_Simulation::SIG_SimulationParameters &simulationParameter,
                      const SIGEL_Environment::SIG_Environment &environment,
                      const SIG_GPParameter &gpParameter );

    QList<Finding> check() const;

  private:

    void checkLimitCannotHoldDrive( QList<Finding> &findings ) const;
    void checkSenseWithoutSensors( QList<Finding> &findings ) const;
    void checkStartOutsideRange( QList<Finding> &findings ) const;
    bool startsOutsideRange( double minimum, double maximum, double initial ) const;

    bool checkLinkWithoutMass( QList<Finding> &findings, const SIGEL_Simulation::SIG_StartPose &startPose ) const;
    void checkStrokeThrowsRobot( QList<Finding> &findings, const SIGEL_Simulation::SIG_StartPose &startPose ) const;
    void checkAxisOffEdge( QList<Finding> &findings, const SIGEL_Simulation::SIG_StartPose &startPose ) const;
    void checkLinksOverlap( QList<Finding> &findings, const SIGEL_Simulation::SIG_StartPose &startPose ) const;
    void checkStartHeight( QList<Finding> &findings, const SIGEL_Simulation::SIG_StartPose &startPose ) const;
    void checkStepForGroundContact( QList<Finding> &findings, const SIGEL_Simulation::SIG_StartPose &startPose ) const;
    void checkStepForJoints( QList<Finding> &findings, const SIGEL_Simulation::SIG_StartPose &startPose ) const;
    void checkHoldsStartPose( QList<Finding> &errors, QList<Finding> &warnings, QList<Finding> &suggestions, const SIGEL_Simulation::SIG_StartPose &startPose ) const;

    // The distance from an axis to the second nearest vertex of a link.
    double offAxis( const SIGEL_Simulation::SIG_StartPose &startPose, const SIGEL_Robot::SIG_Link *link, SIG_Vector axisPoint, SIG_Vector axis ) const;

    // Measured on the seven shipped robots and one rejected model. A joint that is a pin in a fork is above it and sound.
    static constexpr double axisOffEdgePercent = 1.0;
    // The shipped robots and a robot of pins in forks overlap by 0.03 % at most; a rejected model by 5.8 %.
    static constexpr double overlapPercent = 1.0;
    static constexpr int overlapSamples = 20000;
    // Of the seven shipped robots, the two above 8 start heights are thrown; the highest quiet one is at 6.1.
    static constexpr double throwStartHeights = 8.0;
    // The simulation's floor is at 0 whatever YPLANELEVEL says.
    static constexpr double floorLevel = 0.0;
    // Single bodies go unstable from 0.5; the shipped robots show the first NaN at 1.5.
    static constexpr double groundContactStability = 0.75;
    // With joint friction the simulation goes unstable at 2.785 on every shipped robot; 2.5 leaves 10 %.
    static constexpr double jointStability = 2.5;
    static constexpr double jointFrictionInstability = 2.785;
    // The robot is left alone for this long, a hair above the floor. All ten measured robots rest by then.
    static constexpr double restSeconds = 5.0;
    static constexpr double restClearance = 1e-6;
    // Provisional. Robots that stand turn a joint by 3.9 degrees at most and sink 2 % of their height;
    // two that settle turn 19 to 21 degrees; the one that collapses turns 52 degrees and sinks 43 %.
    static constexpr double settlesDegrees = 10.0;
    static constexpr double collapsesDegrees = 30.0;
    static constexpr double collapsesSinkPercent = 25.0;

    const SIGEL_Robot::SIG_Robot &robot;
    const SIGEL_Simulation::SIG_SimulationParameters &simulationParameter;
    const SIGEL_Environment::SIG_Environment &environment;
    const SIG_GPParameter &gpParameter;

  };

}

#endif // SIGEL_GP_SIG_ROBOTCHECKER_H
