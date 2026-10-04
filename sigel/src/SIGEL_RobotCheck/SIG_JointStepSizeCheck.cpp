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
#include "SIGEL_RobotCheck/SIG_JointStepSizeCheck.h"

#include "SIGEL_RobotCheck/SIG_JointInertia.h"
#include "SIGEL_Tools/SIG_Exception.h"

#include <algorithm>
#include <cmath>

SIGEL_RobotCheck::SIG_JointStepSizeCheck::SIG_JointStepSizeCheck( const SIGEL_RobotCheck::SIG_RobotStartPose &startPose,
                                                                  const SIGEL_Environment::SIG_Environment &environment,
                                                                  const SIGEL_Simulation::SIG_SimulationParameters &simulationParameter )
  : startPose( startPose ),
    environment( environment ),
    simulationParameter( simulationParameter )
{
}

QList<SIGEL_RobotCheck::SIG_RobotIssue> SIGEL_RobotCheck::SIG_JointStepSizeCheck::issues() const
{
  QList<SIG_RobotIssue> issues;

  // The check computes with the links' masses.
  if ( !startPose.everyLinkHasMass() )
    return issues;

  // The limits for the step size are measured for Runge-Kutta 4 only.
  if ( simulationParameter.getDynaMechsIntegrator() != SIGEL_Simulation::SIG_SimulationParameters::RungeKutta4 )
    return issues;

  // The joints' inertia is defined only for a robot whose joints all turn.
  if ( startPose.getRobot().getJoints().isEmpty() )
    return issues;

  for ( SIGEL_Robot::SIG_Joint *joint : startPose.getRobot().getJoints() )
    {
      if ( joint->getJointType() != SIGEL_Robot::SIG_Joint::tRotationalJoint )
        return issues;
    }

  try
    {
      SIG_JointInertia jointInertia( startPose.getRobot(), environment, simulationParameter );
      double inertia = jointInertia.getSmallest();

      // The rates at which joint friction, the limit damper and the limit spring act on the joints.
      double frictionRate = simulationParameter.getJointFrictionU_c() / inertia;
      double damperRate = simulationParameter.getJointLimitsB_damper() / inertia;
      double springRate = std::sqrt( simulationParameter.getJointLimitsK_spring() / inertia );

      double fastestRate = std::max( { frictionRate, damperRate, springRate } );
      if ( !( fastestRate > 0 ) )
        return issues;

      double suggestedStep = jointStability / fastestRate;
      if ( simulationParameter.getStepSize() <= suggestedStep )
        return issues;

      QString limitedBy = "the joint-limit spring";
      QString effect = "Runs may still work; the spring acts only past a limit.";
      QString advice = "Lower the step size or the joint-limit spring.";
      if ( fastestRate == damperRate )
        {
          limitedBy = "the joint-limit damper";
          effect = "Runs may still work; the damper acts only past a limit.";
          advice = "Lower the step size or the joint-limit damper.";
        }
      if ( fastestRate == frictionRate )
        {
          limitedBy = "joint friction";
          effect = "The step is close to the limit: the simulation works but has no margin.";
          if ( simulationParameter.getStepSize() * frictionRate > jointFrictionInstability )
            effect = "The simulation breaks down, and programs score 0.";
          advice = "Lower the step size or the joint friction.";
        }

      issues.append( SIG_RobotIssue{ SIG_RobotIssue::tJointStepSize, SIG_RobotIssue::tSuggestion, "Step size for the joints", QString(),
                                     QString( "Step size %1 is above the suggested %2 for the joints, limited by %3. " ).arg( simulationParameter.getStepSize() ).arg( suggestedStep, 0, 'g', 3 ).arg( limitedBy ) + effect,
                                     advice } );
    }
  catch ( SIGEL_Tools::SIG_Exception &e )
    {
      // The message's later lines name the source file that threw.
      issues.append( SIG_RobotIssue::cannotBeSimulated( e.getMessage().section( '\n', 0, 0 ) ) );
    }

  return issues;
}
