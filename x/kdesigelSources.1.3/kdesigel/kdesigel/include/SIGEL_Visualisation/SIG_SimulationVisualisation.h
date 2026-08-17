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

#include "SIGEL_Visualisation/SIG_Visualisation.h"
#include "SIGEL_Visualisation/SIG_RobotRenderer.h"
#include "SIGEL_Visualisation/SIG_EnvironmentRenderer.h"
#include "SIGEL_Visualisation/SIG_RenderRecorder.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_Simulation/SIG_Simulation.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"

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
      void makeTimeSteps(int noOfTimeSteps)
	throw (SIGEL_Simulation::SIG_SimulationCannotSolveException);

      QTime getSimulationTime() const;

      DL_vector getRobotPosition() const;

      DL_matrix getRobotRotation() const;

      bool exportToPovray( QString includeFilename,
			   QString fileName );

      bool createPovrayIncludeFile( QString filename,
				    double aspectRatio );

    private:

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
       * The SIG_Environment object that should take part
       * in the simulation.
       */
      SIGEL_Environment::SIG_Environment const &environment;

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

      /**
       * The SIG_SimulationParameter that should be used in
       * the simulation.
       */
      SIGEL_Simulation::SIG_SimulationParameters const simulationParameter;

      /**
       * The SIG_Program object that should take part in
       * the simulation.
       */
      SIGEL_Program::SIG_Program const &program;

      int frameCounter;

      int robotPathPointAddingRate;

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
