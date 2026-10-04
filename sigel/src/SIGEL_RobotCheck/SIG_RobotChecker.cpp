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
#include "SIGEL_RobotCheck/SIG_RobotChecker.h"

#include "SIGEL_RobotCheck/SIG_DriveStrengthCheck.h"
#include "SIGEL_RobotCheck/SIG_GroundStepSizeCheck.h"
#include "SIGEL_RobotCheck/SIG_IntegratorCheck.h"
#include "SIGEL_RobotCheck/SIG_JointAxisCheck.h"
#include "SIGEL_RobotCheck/SIG_JointStartCheck.h"
#include "SIGEL_RobotCheck/SIG_JointStepSizeCheck.h"
#include "SIGEL_RobotCheck/SIG_LimitHoldsDriveCheck.h"
#include "SIGEL_RobotCheck/SIG_LinkMassCheck.h"
#include "SIGEL_RobotCheck/SIG_LinkOverlapCheck.h"
#include "SIGEL_RobotCheck/SIG_RobotStartPose.h"
#include "SIGEL_RobotCheck/SIG_SimulationCheck.h"
#include "SIGEL_RobotCheck/SIG_StandingCheck.h"
#include "SIGEL_RobotCheck/SIG_StartHeightCheck.h"

#include <algorithm>

SIGEL_RobotCheck::SIG_RobotChecker::SIG_RobotChecker( const SIGEL_Robot::SIG_Robot &robot,
                                              const SIGEL_Simulation::SIG_SimulationParameters &simulationParameter,
                                              const SIGEL_Environment::SIG_Environment &environment )
  : robot( robot ),
    simulationParameter( simulationParameter ),
    environment( environment )
{
}

QList<SIGEL_RobotCheck::SIG_RobotIssue> SIGEL_RobotCheck::SIG_RobotChecker::check() const
{
  SIG_RobotStartPose startPose( robot, environment, simulationParameter );

  QList<SIG_RobotIssue> issues;
  issues += SIG_SimulationCheck( startPose ).issues();
  issues += SIG_LimitHoldsDriveCheck( robot, simulationParameter ).issues();
  issues += SIG_DriveStrengthCheck( startPose, environment ).issues();
  issues += SIG_JointAxisCheck( startPose ).issues();
  issues += SIG_LinkOverlapCheck( startPose ).issues();
  issues += SIG_JointStartCheck( robot ).issues();
  issues += SIG_StartHeightCheck( startPose, environment ).issues();
  issues += SIG_GroundStepSizeCheck( startPose, environment, simulationParameter ).issues();
  issues += SIG_JointStepSizeCheck( startPose, environment, simulationParameter ).issues();
  issues += SIG_IntegratorCheck( simulationParameter ).issues();
  issues += SIG_LinkMassCheck( startPose ).issues();
  issues += SIG_StandingCheck( robot, startPose, environment, simulationParameter ).issues();

  std::stable_sort( issues.begin(), issues.end() );
  return issues;
}
