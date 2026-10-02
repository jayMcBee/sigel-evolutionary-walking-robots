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
#ifndef SIGEL_SIMULATION_SIG_RESTRUN_H
#define SIGEL_SIMULATION_SIG_RESTRUN_H

#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"
#include "SIGEL_Tools/SIG_Matrix.h"

#include <qstring.h>

namespace SIGEL_Simulation
{

  /**
   * A simulation of a robot that no program moves. The constructor lifts the
   * robot by the given height, runs for the given time and keeps how far the
   * robot left its start pose.
   */
  class SIG_RestRun
  {

  public:

    SIG_RestRun( const SIGEL_Robot::SIG_Robot &robot,
                 const SIGEL_Environment::SIG_Environment &environment,
                 const SIG_SimulationParameters &simulationParameter,
                 double lift,
                 double seconds );

    // The largest turn of a joint away from its start angle, in degrees, and the joint that made it.
    double getLargestTurn() const;
    QString getLoosestJoint() const;

    // How far the root link is below its start height at the end.
    double getSink() const;

    // Whether the robot's position stopped being a number, and after how many seconds.
    bool hasBrokenDown() const;
    double getBrokeAfter() const;

  private:

    double turnBetween( SIG_Matrix left, SIG_Matrix right, SIG_Matrix leftAtStart, SIG_Matrix rightAtStart ) const;

    double largestTurn;
    QString loosestJoint;
    double sink;
    double brokeAfter;

  };

}

#endif // SIGEL_SIMULATION_SIG_RESTRUN_H
