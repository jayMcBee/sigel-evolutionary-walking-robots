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
#ifndef SIGEL_ROBOTCHECK_SIG_ROBOTSTARTPOSE_H
#define SIGEL_ROBOTCHECK_SIG_ROBOTSTARTPOSE_H

#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_RobotCheck/SIG_LinkVolume.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"
#include "SIGEL_Tools/SIG_Matrix.h"
#include "SIGEL_Tools/SIG_Vector.h"

#include <QList>
#include <qstring.h>

namespace SIGEL_RobotCheck
{

  /**
   * A robot as the simulation has it before the first step: a copy of the
   * robot that is prepared for the simulation, each link's mass, and where
   * the simulation puts each link. The simulation cannot take every robot;
   * then only canBeSimulated() and getRefusal() may be asked.
   */
  class SIG_RobotStartPose
  {

  public:

    // The floor of the simulation is at height 0, whatever YPLANELEVEL says.
    static constexpr double floorLevel = 0.0;

    SIG_RobotStartPose( const SIGEL_Robot::SIG_Robot &robot,
                        const SIGEL_Environment::SIG_Environment &environment,
                        const SIGEL_Simulation::SIG_SimulationParameters &simulationParameter );

    bool canBeSimulated() const;

    // Why the simulation cannot take the robot; empty if it can.
    QString getRefusal() const;

    // False too for a robot that cannot be simulated.
    bool everyLinkHasMass() const;

    const SIGEL_Robot::SIG_Robot &getRobot() const;

    // The links' masses, in the order of the robot's links.
    const QList<double> &getMasses() const;

    SIG_Vector getPosition( const SIGEL_Robot::SIG_Link *link ) const;
    SIG_Matrix getOrientation( const SIGEL_Robot::SIG_Link *link ) const;

    // A point of a link, in the world.
    SIG_Vector toWorld( const SIGEL_Robot::SIG_Link *link, SIG_Vector point ) const;

    SIG_LinkVolume getLinkVolume( const SIGEL_Robot::SIG_Link *link ) const;

    void getExtent( int axis, double &lowest, double &highest ) const;

    // The largest of the robot's three extents.
    double getSize() const;

    // The start height at which the robot's lowest vertex is on the floor.
    double getSafeStartHeight() const;

  private:

    SIGEL_Robot::SIG_Robot startRobot;
    QList<double> masses;
    QList<SIG_Vector> positions;
    QList<SIG_Matrix> orientations;
    QString refusal;
    double startHeight;

  };

}

#endif // SIGEL_ROBOTCHECK_SIG_ROBOTSTARTPOSE_H
