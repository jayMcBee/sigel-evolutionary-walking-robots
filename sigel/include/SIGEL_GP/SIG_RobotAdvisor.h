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
#ifndef SIGEL_GP_SIG_ROBOTADVISOR_H
#define SIGEL_GP_SIG_ROBOTADVISOR_H

#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_GP/SIG_GPParameter.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"
#include "SIGEL_Tools/SIG_Matrix.h"
#include "SIGEL_Tools/SIG_Vector.h"

#include <QList>
#include <qstring.h>
#include <qstringlist.h>

namespace SIGEL_GP
{

  /**
   * Reads a robot and its settings and lists what will probably make an
   * evolution fail or mislead. Every finding is advice; none stops a run.
   */
  class SIG_RobotAdvisor
  {

  public:

    // An error is a robot that cannot run at all.
    enum Kind { tError, tWarning, tSuggestion };

    /**
     * One piece of advice. check is the number of the check that found it,
     * part the robot parts it is about, value the worst that was computed and
     * limit what that was compared with. Check 1 gives both in degrees, checks
     * 2 and 7 as heights, check 4 in percent of the robot's size, check 6 in
     * the joint's own unit, check 8 in seconds. Check 0 is a robot that cannot
     * be simulated, check 10 a link without mass.
     */
    struct Finding { int check; Kind kind; QString part; double value; double limit; QString text; };

    SIG_RobotAdvisor( const SIGEL_Robot::SIG_Robot &robot,
                      const SIGEL_Simulation::SIG_SimulationParameters &simulationParameter,
                      const SIGEL_Environment::SIG_Environment &environment,
                      const SIG_GPParameter &gpParameter );

    QList<Finding> advise() const;

  private:

    // Where the simulation puts each link at its start, by link number.
    struct StartPose { QList<SIG_Vector> positions; QList<SIG_Matrix> orientations; };

    void adviseLimitCannotHoldDrive( QList<Finding> &findings ) const;
    void adviseSenseWithoutSensors( QList<Finding> &findings ) const;
    void adviseStartOutsideRange( QList<Finding> &findings ) const;
    bool startsOutsideRange( double minimum, double maximum, double initial ) const;

    // These take a copy of the robot that is prepared for the simulation.
    bool adviseLinkWithoutMass( QList<Finding> &findings, const QList<double> &masses, const SIGEL_Robot::SIG_Robot &startRobot ) const;
    void adviseStrokeThrowsRobot( QList<Finding> &findings, const QList<double> &masses, const SIGEL_Robot::SIG_Robot &startRobot ) const;
    void adviseAxisOffEdge( QList<Finding> &findings, const SIGEL_Robot::SIG_Robot &startRobot, const StartPose &startPose ) const;
    void adviseStartHeight( QList<Finding> &findings, const SIGEL_Robot::SIG_Robot &startRobot, const StartPose &startPose ) const;
    void adviseStepForGroundContact( QList<Finding> &findings, const QList<double> &masses, const SIGEL_Robot::SIG_Robot &startRobot ) const;

    StartPose startPoseOf( const SIGEL_Robot::SIG_Robot &startRobot ) const;
    SIG_Vector atStart( const StartPose &startPose, const SIGEL_Robot::SIG_Link *link, SIG_Vector point ) const;
    double offAxis( const StartPose &startPose, const SIGEL_Robot::SIG_Link *link, SIG_Vector axisPoint, SIG_Vector axis ) const;
    double sizeAtStart( const SIGEL_Robot::SIG_Robot &startRobot, const StartPose &startPose ) const;

    // Measured on the seven shipped robots and one rejected model.
    static constexpr double axisOffEdgePercent = 1.0;
    // Of the seven shipped robots, the two above 8 start heights are thrown; the highest quiet one is at 6.1.
    static constexpr double throwStartHeights = 8.0;
    // The simulation's floor is at 0 whatever YPLANELEVEL says.
    static constexpr double floorLevel = 0.0;
    // Single bodies go unstable from 0.5; the shipped robots show the first NaN at 1.5.
    static constexpr double groundContactStability = 0.75;

    const SIGEL_Robot::SIG_Robot &robot;
    const SIGEL_Simulation::SIG_SimulationParameters &simulationParameter;
    const SIGEL_Environment::SIG_Environment &environment;
    const SIG_GPParameter &gpParameter;

  };

}

#endif // SIGEL_GP_SIG_ROBOTADVISOR_H
