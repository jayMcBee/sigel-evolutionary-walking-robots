/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler

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
#include "compat/q2compat.h"
#include "SIGEL_Simulation/SIG_DynaMoSimulationData.h"

SIGEL_Simulation::SIG_DynaMoSimulationData::SIG_DynaMoSimulationData( SIGEL_Robot::SIG_Robot const& robot,
								      SIGEL_Environment::SIG_Environment const& environment,
								      SIGEL_Simulation::SIG_SimulationParameters const& simulationParameter)
  : SIG_SimulationData( robot, environment, simulationParameter ),
    dynaSystem( environment, simulationParameter)
{
  dynaSystem.floorMaterial=robot.lookupMaterial(environment.getFloorMaterialName());
  for (SIGEL_Robot::SIG_Link *l : robot.getLinks())
    dynaSystem.newLink( *l );
  for (SIGEL_Robot::SIG_Joint *j : robot.getJoints())
    dynaSystem.newJoint( *j );
  for (SIGEL_Robot::SIG_Sensor *s : robot.getSensors())
    dynaSystem.newSensor( *s );
  for (SIGEL_Robot::SIG_Drive *d : robot.getDrives())
    dynaSystem.newDrive( *d );
};

void SIGEL_Simulation::SIG_DynaMoSimulationData::setNewFrame( bool newValue )
{
  dynaSystem.newFrame = newValue;
};

void SIGEL_Simulation::SIG_DynaMoSimulationData::simulationProgress()
{
  dynaSystem.doNewFrame();
};
