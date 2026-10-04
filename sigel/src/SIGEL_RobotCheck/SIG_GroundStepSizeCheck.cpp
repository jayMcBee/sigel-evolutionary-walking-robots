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
#include "SIGEL_RobotCheck/SIG_GroundStepSizeCheck.h"

#include <algorithm>
#include <cmath>

SIGEL_RobotCheck::SIG_GroundStepSizeCheck::SIG_GroundStepSizeCheck( const SIGEL_RobotCheck::SIG_RobotStartPose &startPose,
                                                                    const SIGEL_Environment::SIG_Environment &environment,
                                                                    const SIGEL_Simulation::SIG_SimulationParameters &simulationParameter )
  : startPose( startPose ),
    environment( environment ),
    simulationParameter( simulationParameter )
{
}

QList<SIGEL_RobotCheck::SIG_RobotIssue> SIGEL_RobotCheck::SIG_GroundStepSizeCheck::issues() const
{
  QList<SIG_RobotIssue> issues;

  // The check computes with the links' masses.
  if ( !startPose.everyLinkHasMass() )
    return issues;

  // The limits for the step size are measured for Runge-Kutta 4 only.
  if ( simulationParameter.getDynaMechsIntegrator() != SIGEL_Simulation::SIG_SimulationParameters::RungeKutta4 )
    return issues;

  const QList<double> &masses = startPose.getMasses();
  if ( masses.isEmpty() )
    return issues;

  int lightest = std::min_element( masses.begin(), masses.end() ) - masses.begin();
  double lightestMass = masses[ lightest ];
  QString lightestLink = startPose.getRobot().getLinks()[ lightest ]->getName();

  // The fastest rate of the ground's penalty springs and dampers on the lightest link.
  double fastestRate = std::max( { std::sqrt( environment.getGroundNormalSpringConstant() / lightestMass ),
                                   std::sqrt( environment.getGroundPlanarSpringConstant() / lightestMass ),
                                   environment.getGroundNormalDamperConstant() / lightestMass,
                                   environment.getGroundPlanarDamperConstant() / lightestMass } );
  double suggestedStep = groundContactStability / fastestRate;

  if ( simulationParameter.getStepSize() > suggestedStep )
    {
      issues.append( SIG_RobotIssue{ SIG_RobotIssue::tGroundStepSize, SIG_RobotIssue::tSuggestion, "Step size for ground contact", lightestLink,
                                     QString( "Step size %1 is above the suggested %2 for ground contact on the lightest link (mass %3). Runs may still work; if they do not, robots fly off or score 0." ).arg( simulationParameter.getStepSize() ).arg( suggestedStep, 0, 'g', 3 ).arg( lightestMass, 0, 'g', 3 ),
                                     "Lower the step size." } );
    }

  return issues;
}
