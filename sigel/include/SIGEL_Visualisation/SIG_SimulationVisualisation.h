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
#ifndef SIGEL_VISUALISATION_SIG_SIMULATIONVISUALISATION_H
#define SIGEL_VISUALISATION_SIG_SIMULATIONVISUALISATION_H

#include <qcolor.h>
#include <QMatrix4x4>
#include <QVector3D>

#include "SIGEL_Visualisation/SIG_Visualisation.h"
#include "SIGEL_Visualisation/SIG_RobotRenderer.h"
#include "SIGEL_Visualisation/SIG_EnvironmentRenderer.h"
#include "SIGEL_Visualisation/SIG_RenderRecorder.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_Simulation/SIG_Simulation.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"

class QOpenGLContext;
class QOpenGLExtraFunctions;
class QOpenGLShaderProgram;

namespace SIGEL_Visualisation
{

  /**
   * This class is used to visualize simulations.
   *
   * Like classes inherited from SIG_GPFitnessFunction, the
   * SIG_SimulationVisualisation contains a SIG_Simulation object
   * and uses a class inherited from the SIG_Recorder class
   * (the SIG_RenderRecorder) to retrieve information from
   * the simulation.
   * Besides the inherited method visualize there is a method
   * makeTimeSteps that effects the progress of the
   * contained simulation and visualisation data structures.
   */
  class SIG_SimulationVisualisation : public SIG_Visualisation
    {
    public:

      /**
       * The constructor of the SIG_SimulationVisualisation.
       *
       * It initializes the references that are used to acces the elements
       * that take part in the simulation/visualisation.
       * It creates the SIG_Simulation and SIG_RenderRecorder objects.
       * It initializes the OpenGL stuff.
       *
       * @param robot The robot used for the simulation.
       * @param environment The environment used for the simulation.
       * @param simulationParameter The simulation parameters used for the simulation.
       * @param program The robot control program used for the simulation.
       */
      SIG_SimulationVisualisation(SIGEL_Robot::SIG_Robot const &robot,
				  SIGEL_Environment::SIG_Environment const &environment,
				  SIGEL_Simulation::SIG_SimulationParameters const &simulationParameter,
				  SIGEL_Program::SIG_Program const &program);

      /**
       * The destructor of the SIG_SimulationVisualisation.
       *
       * Cleans up the OpenGL stuff and deletes the simulation and the renderRecorder.
       */
      ~SIG_SimulationVisualisation();

      void setPointsVisible( bool visible );

      void setPlaneColor( QColor newColor );

      void setShowPlane( bool newShowPlane );

      void setShowGrid( bool newShowGrid );

      void setShowRobotPath( bool newShowRobotPath );

      void setShowShadows( bool newShowShadows );

      /**
       * Returns false when shadow mapping could not be set up; then
       * setShowShadows has no effect.
       */
      bool canShowShadows() const;

      /**
       * This method finally initiates the calling
       * of certain OpenGL rendering commands.
       *
       * They refresh the active framebuffer of the
       * current OpenGL context according to the
       * current state of the simulation and the
       * attribute viewSettings.
       */
      void visualize();

      /**
       * This method lets the simulation progress
       * and updates internal data structures
       * (for example position and rotation of objects).
       *
       * @param noOfTimeSteps The number of timeSteps the simulation should progress about.
       */
      void makeTimeSteps(int noOfTimeSteps);

      QTime getSimulationTime() const;

      DL_vector getRobotPosition() const;

      DL_matrix getRobotRotation() const;

      /**
       * Returns the centre of the box around the robot, where the
       * robot is now. The box is measured at time 0 and moves with
       * the root link.
       */
      DL_vector getRobotCentre() const;

      /**
       * Returns the radius of the sphere around that box.
       */
      double getRobotRadius() const;

      bool exportToPovray( QString includeFilename,
			   QString fileName );

      bool createPovrayIncludeFile( QString filename,
				    double aspectRatio );

    private:

      /**
       * The direction from the scene towards the sun, in world
       * coordinates, not normalised. The sun is a directional light
       * fixed to the world; only its light is shadowed.
       */
      static constexpr QVector3D sunDirection = QVector3D( -0.6f, 1.0f, 0.4f );

      /**
       * The diffuse intensity of the sun, the same for red, green and blue.
       */
      static constexpr float sunIntensity = 0.5f;

      /**
       * The opacity of the plane in the wireframe, hidden lines and
       * points modes.
       */
      static constexpr float unlitPlaneAlpha = 0.3f;

      /**
       * The width and height of the shadow map in texels.
       */
      static constexpr int shadowMapSize = 2048;

      /**
       * The shadow map covers robotRadius times this, because the limbs
       * swing outside the robot's sphere at time 0.
       */
      static constexpr float shadowBoxMargin = 1.1f;

      /**
       * The radius of the filter around each shadow map lookup, in texels.
       */
      static constexpr float shadowFilterRadius = 1.5f;

      /**
       * The GLSL sources of the program that draws the lit plane and
       * robot links with the sun's shadow.
       */
      static char const *const shadowVertexShaderSource;
      static char const *const shadowFragmentShaderSource;

      /**
       * Creates the shadow map, its framebuffer and the program in the
       * current OpenGL context and sets shadowProgram. On a failure it
       * prints the error to SIG_IO::cerr, frees what it created and
       * leaves shadowProgram nullptr. A robot without vertices gets no
       * shadow and no message.
       */
      void initShadowMapping();

      /**
       * Draws the robot links into the shadow map, seen from the sun
       * around the robot. Returns the sun's projection times its view.
       */
      QMatrix4x4 renderShadowMap();

      /**
       * Draws the scene lit, with the sun's shadow on the plane and the
       * robot links. Grid, robot path and anchor points are drawn as
       * without shadows.
       *
       * @param lightMatrix The matrix renderShadowMap returned for
       *        this frame.
       * @pre The camera's modelview matrix is set.
       */
      void renderShadowedScene( QMatrix4x4 const &lightMatrix );

      /**
       * The context the shadow objects below belong to.
       */
      QOpenGLContext *shadowContext;

      QOpenGLExtraFunctions *glFunctions;

      /**
       * The program that draws with shadows, or nullptr when shadow
       * mapping could not be set up.
       */
      QOpenGLShaderProgram *shadowProgram;

      GLuint shadowFramebuffer;

      GLuint shadowTexture;

      bool showShadows;

      /**
       * The SIG_EnvironmentRenderer object that is
       * responsible for the rendering of the environment.
       *
       * While this SIG_SimulationVisualisation object
       * takes care of all OpenGL related things to be done
       * (framebuffer initialization and setting of
       * OpenGL state variables for example),
       * the final rendering of the environment
       * is done by the environmentRenderer.
       */
      SIG_EnvironmentRenderer environmentRenderer;

      /**
       * The SIG_RobotRenderer object that is
       * responsible for the rendering of the robot.
       *
       * While this SIG_SimulationVisualisation object
       * takes care of all OpenGL related things to be done
       * (framebuffer initialization and setting of
       * OpenGL state variables for example),
       * the final rendering of the robot
       * is done by the robotRenderer.
       */
      SIG_RobotRenderer robotRenderer;

      /**
       * The SIG_Robot object that should take part
       * in the simulation.
       */
      SIGEL_Robot::SIG_Robot const &robot;

      /**
       * The pointer to the SIG_RenderRecorder object that
       * is used to retrieve acutal information about the
       * elements of the simulation.
       *
       * Is created in the constructor.
       */
      SIG_RenderRecorder *renderRecorder;

      /**
       * The pointer to the SIG_Simulation that should
       * be visualized.
       * Is created in the constructor.
       */
      SIGEL_Simulation::SIG_Simulation *simulation;

      int frameCounter;

      int robotPathPointAddingRate;

      /**
       * The centre of the robot's box at time 0, in the
       * root link's frame.
       */
      DL_vector centreInRootLink;

      /**
       * The radius of the sphere around the robot's box at time 0.
       */
      double robotRadius;

      /**
       * Reads the actual positions and rotations of the
       * robot's links from the renderRecorder and writes them in the
       * appropriate elements of the attribute sceneObjects of the robotRenderer.
       *
       * Is called by the constructor for setting up the initial robot.
       * Is called in makeTimeSteps.
       */
      void updateRobotLinks();

      /**
       * Reads the actual positions and rotations of the robot's links
       * and sets the according values of the appropriate robot points.
       *
       * Is called by the constructor for setting up the initial robot.
       * Is called in makeTimeSteps.
       */
      void updateRobotPoints();

    };

}

#endif // SIGEL_VISUALISATION_SIG_SIMULATIONVISUALISATION_H
