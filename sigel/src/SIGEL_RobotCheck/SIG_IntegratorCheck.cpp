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
#include "SIGEL_RobotCheck/SIG_IntegratorCheck.h"

SIGEL_RobotCheck::SIG_IntegratorCheck::SIG_IntegratorCheck( const SIGEL_Simulation::SIG_SimulationParameters &simulationParameter )
  : simulationParameter( simulationParameter )
{
}

QList<SIGEL_RobotCheck::SIG_RobotIssue> SIGEL_RobotCheck::SIG_IntegratorCheck::issues() const
{
  QList<SIG_RobotIssue> issues;

  if ( simulationParameter.getDynaMechsIntegrator() != SIGEL_Simulation::SIG_SimulationParameters::RungeKutta4 )
    issues.append( SIG_RobotIssue{ SIG_RobotIssue::tIntegrator, SIG_RobotIssue::tSuggestion, "Step size not checked", QString(),
                                   "The limits for the step size are measured for the integrator Runge-Kutta 4 only. Another integrator is selected, so the two step size checks did not run.",
                                   "Select Runge-Kutta 4, or find a step size that works by trial." } );

  return issues;
}
