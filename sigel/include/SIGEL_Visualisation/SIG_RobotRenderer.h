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
#ifndef SIGEL_VISUALISATION_SIG_ROBOTRENDERER_H
#define SIGEL_VISUALISATION_SIG_ROBOTRENDERER_H

#include "SIGEL_Visualisation/SIG_Renderer.h"
#include "SIGEL_Robot/SIG_Robot.h"

namespace SIGEL_Visualisation
{

  /**
   * Draws a robot's links and anchor points, and finds the screen position of each point's
   * floating text.
   */
  class SIG_RobotRenderer : public SIG_Renderer
  {
  public:
    /**
     * Creates a scene object for each link and each anchor point, builds their display
     * lists, and hides the points.
     *
     * @pre An OpenGL context is current.
     */
    SIG_RobotRenderer(SIGEL_Robot::SIG_Robot const &robot);

    /**
     * Draws the links, then the anchor points.
     *
     * @pre buildDisplayLists has run.
     */
    void render() override;

    /**
     * Calls the display lists of the robot's links only.
     * No anchor points and no feedback for the floating texts.
     *
     * @pre buildDisplayLists has run.
     */
    void renderLinks();

    /**
     * Calls the display lists of the robot's anchor points and
     * updates the positions of their floating texts.
     *
     * @pre buildDisplayLists has run.
     */
    void renderPoints();

    SIGEL_Robot::SIG_Robot const &getRobot() const { return robot; }

    void setPointsVisible( bool visible );

  private:

    /**
     * Records each link's geometry and each anchor point in its display list, and creates
     * each point's scene object and floating text. The links come first, each at its link
     * number; the points follow.
     */
    void buildDisplayLists() override;    

    SIGEL_Robot::SIG_Robot const &robot;
  };

}

#endif // SIGEL_VISUALISATION_SIG_ROBOTRENDERER_H
