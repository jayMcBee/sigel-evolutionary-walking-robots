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
	 * Shows a running simulation. It runs its own SIG_Simulation, reads the links' positions
	 * through a SIG_RenderRecorder, and draws the environment and the robot.
	 */
	class SIG_SimulationVisualisation : public SIG_Visualisation
	{
	public:

		/**
		 * Creates the simulation, the renderers and the shadow mapping.
		 *
		 * @pre An OpenGL context is current.
		 */
		SIG_SimulationVisualisation(SIGEL_Robot::SIG_Robot const &robot,
		                            SIGEL_Environment::SIG_Environment const &environment,
		                            SIGEL_Simulation::SIG_SimulationParameters const &simulationParameter,
		                            SIGEL_Program::SIG_Program const &program);

		/**
		 * Frees the shadow mapping objects.
		 *
		 * @pre The constructor's OpenGL context is current. If it is not, the shadow map and
		 *      the display lists stay until that context goes.
		 */
		~SIG_SimulationVisualisation() override;

		void setPointsVisible( bool visible );

		void setPlaneColor( QColor newColor );

		void setGridColor( QColor newColor );

		void setShowPlane( bool newShowPlane );

		void setShowGrid( bool newShowGrid );

		void setShowRobotPath( bool newShowRobotPath );

		void setShowShadows( bool newShowShadows );

		/**
		 * Returns false when shadow mapping could not be set up; then
		 * setShowShadows has no effect.
		 */
		bool canShowShadows() const { return shadowProgram != nullptr; }

		/**
		 * Draws the simulation as it is now, with the camera and render mode in viewSettings.
		 */
		void visualize() override;

		/**
		 * Runs the simulation noOfTimeSteps steps on, adds robot path points every 0.5
		 * simulated seconds, and moves the drawn robot to its new place.
		 */
		void makeTimeSteps(int noOfTimeSteps);

		double getCurrentSimulationSeconds() const { return renderRecorder.currentSimulationSeconds; }

		SIG_Vector getRobotPosition() const;

		SIG_Matrix getRobotRotation() const;

		/** The centre of the robot's box at time 0, carried along with the root link. */
		SIG_Vector getRobotCentre() const;

		/**
		 * Returns the radius of the sphere around that box.
		 */
		double getRobotRadius() const { return robotRadius; }

		SIG_EnvironmentRenderer const &getEnvironmentRenderer() const { return environmentRenderer; }

		SIG_RobotRenderer const &getRobotRenderer() const { return robotRenderer; }

		/** Towards the sun, in world coordinates, not normalised; the only light that casts shadows. */
		static constexpr QVector3D sunDirection = QVector3D( -0.6f, 1.0f, 0.4f );

	private:

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

		SIG_EnvironmentRenderer environmentRenderer;

		SIG_RobotRenderer robotRenderer;

		SIGEL_Robot::SIG_Robot const &robot;

		/**
		 * The recorder that holds the current position and rotation
		 * of each link of the simulation.
		 */
		SIG_RenderRecorder renderRecorder;

		/**
		 * The simulation that is shown. It keeps a reference to
		 * renderRecorder, so it is declared after it.
		 */
		SIGEL_Simulation::SIG_Simulation simulation;

		int frameCounter;

		int robotPathPointAddingRate;

		/**
		 * The centre of the robot's box at time 0, in the
		 * root link's frame.
		 */
		SIG_Vector centreInRootLink;

		/**
		 * The radius of the sphere around the robot's box at time 0.
		 */
		double robotRadius;

		/**
		 * Copies each link's position and rotation from renderRecorder to its scene object in
		 * robotRenderer.
		 */
		void updateRobotLinks();

		/**
		 * Moves the robot's anchor points with their links.
		 */
		void updateRobotPoints();
	};

}

#endif // SIGEL_VISUALISATION_SIG_SIMULATIONVISUALISATION_H
