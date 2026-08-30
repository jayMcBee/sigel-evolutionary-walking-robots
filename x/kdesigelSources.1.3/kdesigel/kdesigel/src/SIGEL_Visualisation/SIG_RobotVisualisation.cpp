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
#include "SIGEL_Visualisation/SIG_RobotVisualisation.h"

namespace SIGEL_Visualisation
{

  SIG_RobotVisualisation::SIG_RobotVisualisation(SIGEL_Robot::SIG_Robot const &robot)
    : SIG_Visualisation(), robotRenderer(robot)
  {
    for ( SIGEL_Robot::SIG_Link *linkPtr : robot.getLinks() )
      {
	SIGEL_Robot::SIG_Link &actLink = *linkPtr;

	int number = actLink.getNumber();

	DL_vector position;
	DL_matrix rotation;
	actLink.getInitialLocation( position, rotation );

	robotRenderer.sceneObjects[number]->setPosition(position);
	robotRenderer.sceneObjects[number]->setRotation(rotation);

      };

    for (int i = robot.getLinks().size(); i < robotRenderer.sceneObjects.size(); i++)
      {
	int linkNumber = robotRenderer.sceneObjects[ i ]->getNumber();

	DL_vector position = robotRenderer.sceneObjects[ linkNumber ]->getPosition();
	DL_matrix rotation = robotRenderer.sceneObjects[ linkNumber ]->getRotation();

	robotRenderer.sceneObjects[ i ]->setPosition( position );
	robotRenderer.sceneObjects[ i ]->setRotation( rotation );
      };

  };

  SIG_RobotVisualisation::~SIG_RobotVisualisation()
  { };

  void SIG_RobotVisualisation::visualize()
  {
    SIG_Visualisation::visualize();

    robotRenderer.render();
  };

}
