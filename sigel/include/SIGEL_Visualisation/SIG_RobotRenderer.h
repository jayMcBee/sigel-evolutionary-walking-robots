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
   * This classes task is the rendering of SIG_Robot objects.
   */
  class SIG_RobotRenderer : public SIG_Renderer
  {
  public:
    /**
     * The constructor of the SIG_RobotRenderer.
     *
     * It reads the supplied SIG_Robot, fills the inherited
     * QList sceneObjects and calls the method buildDisplayLists.
     *
     * @param environment The environment that should be rendered.
     */
    SIG_RobotRenderer(SIGEL_Robot::SIG_Robot const &robot);

    /**
     * The destructor of the SIG_EnvironmentRenderer.
     *
     * It cleans up the OpenGL stuff like the display lists.
     */
    ~SIG_RobotRenderer();

    /**
     * Calls the inherited method renderSceneObjects and
     * eventually does additional rendering like drawing
     * link identifiers onto the particular links.
     *
     * @pre The display lists are builded.
     */
    void render();

    /**
     * Calls the display lists of the robot's links only.
     * No anchor points and no feedback for the floating texts.
     *
     * @pre The display lists are builded.
     */
    void renderLinks();

    /**
     * Calls the display lists of the robot's anchor points and
     * updates the positions of their floating texts.
     *
     * @pre The display lists are builded.
     */
    void renderPoints();

    QString exportToPovray();

    QString createPovrayDeclarations();

    void setPointsVisible( bool visible );

  private:

    /**
     * Reads the robot attribute and uses
     * its properties like geometry of objects to
     * build display lists.
     */
    void buildDisplayLists();    

    /**
     * The SIG_Robot object that should be rendered
     * by this SIG_RobotRenderer.
     */
    SIGEL_Robot::SIG_Robot const &robot;

  };

}

#endif // SIGEL_VISUALISATION_SIG_ROBOTRENDERER_H
