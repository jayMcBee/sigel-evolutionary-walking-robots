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
#ifndef SIGEL_SLAVEGUI_SIG_SIMULATIONVISUALISATIONWIDGET_H
#define SIGEL_SLAVEGUI_SIG_SIMULATIONVISUALISATIONWIDGET_H

#include <QTimer>
#include <QColor>

#include "SIGEL_CommonGUI/SIG_VisualisationWidget.h"

#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"
#include "SIGEL_Program/SIG_Program.h"

// namespace SIGEL_SlaveGUI
// {

  /**
   * This widget finally contains the OpenGL framebuffers in which
   * is rendered by the SIG_SimulationVisualisation object that should
   * be referenced by the inherited pointer visualisation.
   *
   * It offers interfaces to the simulation relevant settings and functions
   * like controlling the simulation or setting the delay between two
   * frames.
   */
  class SIG_SimulationVisualisationWidget : public SIG_VisualisationWidget
    {
      Q_OBJECT

    public:

      /**
       * The constructor of the SIG_SimulationVisualisationWidget.
       *
       * @param parent Pointer to this widget's parent.
       * @param name This widget's name.
       * @param f The widget flag of this widget.
       */
      SIG_SimulationVisualisationWidget( QWidget *parent=0,
					 char const *name=0,
					 Qt::WindowFlags f = Qt::WindowFlags() );

      /**
       * The destructor of the SIG_SimulationVisualisationWidget.
       */
      ~SIG_SimulationVisualisationWidget();

      /**
       * Deletes the possibly existing SIG_SimulationVisualisation object
       * to which the inherited pointer visualisation points and creates
       * a new one with the supplied arguments.
       *
       * @param rrobot The robot that should take part in the simulation.
       * @param eenvironment The environment that should take part in the simulation.
       * @param ssimulationParameters The parameters of the simulation.
       * @param pprogram The program that should control the robot in the simulation.
       */
      void visualizeThis(SIGEL_Robot::SIG_Robot const &rrobot,
			 SIGEL_Environment::SIG_Environment const &eenvironment,
			 SIGEL_Simulation::SIG_SimulationParameters const &ssimulationParameters,
			 SIGEL_Program::SIG_Program const &pprogram);

      /**
       * Is true if the simulation is in running state.
       */
      bool simulationRunning();

      void resetRecorder();

      /**
       * Returns the camera distance at which the whole robot
       * fits in the view, with a margin around it.
       */
      double getFittingDistance() const;

      /**
       * Stops the running simulation while a modal dialog is open.
       * Does nothing if the simulation is not running.
       */
      void pauseForDialog();

      /**
       * Starts the simulation again if pauseForDialog stopped it.
       */
      void resumeAfterDialog();

    public slots:

      void setShowAncorPoints( int state );

      void choosePlaneColor();

      void setShowPlane( int state );

      void setShowGrid( int state );

      void setShowRobotPath( int state );

      /**
       * Toggles between playing and pause state of the simulation.
       */
      void slotStartSimulation();

      /**
       * Deletes the actual simulation run and creates a new one.
       */
      void slotStopSimulation();

      /**
       * Effects the progress of the simulation about one timestep.
       */
      void slotStepSimulation();

      /**
       * Effects the progress of the simulation about 5 seconds.
       */
      void slotFForwardSimulation();

      /**
       * Sets the delay between to simulated and visualized timesteps.
       *
       * Has only effects when the simulation in running.
       *
       * @param frameDelay The new framedelay.
       */
      void slotSetFrameDelay(int fframeDelay);

      void slotSetTraceRobot( bool newValue );

      void slotNavigateForward();

      void slotNavigateBackward();

      void slotNavigateRight();

      void slotNavigateLeft();

      void slotNavigateDown();

      void slotNavigateUp();

      void slotNavigateCenter();

      /**
       * Sets whether the simulation shall be recorded.
       */
      void slotRecordClicked();

      /**
       * Used to alter the movie settings.
       */
      void slotAlterMovieSettingsClicked();

    private:

      /**
       * The timer that controls the progress of the simulation.
       */
      QTimer *simulationTimer;

      /**
       * The time in ms to wait between to simulationframes.
       */
      int frameDelay;

      /**
       * True while a modal dialog holds the simulation paused.
       */
      bool pausedForDialog;

      /**
       * The number of timesteps that equal 5 seconds.
       */
      int noOfFFSteps;

      bool traceRobot;

      QColor planeColor;

      /**
       * Indicates whether simulation shall be recorded.
       */
      bool record;

      bool useLeadingZeros;

      int movieWidth;

      int movieHeight;

      int movieFrequency;

      int movieMaxFrames;

      int movieQuality;

      bool keepRatio;

      bool cropImage;

      /**
       * This is the name for the next frame to be saved (initially 0).
       *
       * currentFrameName does not have to be equal to current frame as it is possible to save every n-th frame.
       */
      int currentFrameName;

      /**
       * The number of the actual frame (initially 0).
       *
       * When the stop button is hit this value has to be resetted to 0.
       */
      int currentFrame;

      QString movieDirectory;

      QString movieFilePrefix;

      QString fileFormat;

      void paintGL();

      /**
       * Let the simulation progress about the supplied
       * number of timesteps.
       *
       * @param noOfSteps The number of timesteps to progress.
       */
      void makeTimeSteps(int noOfSteps);

			/**
			 * Calls the QT-Function renderPixmap(), which creates a new pixmap on the fly.
   		 * This will be deleted when it gets out of scope;
       * Returns whether saving was successful.
			 */
			bool callRenderPixMap( QString inFName );


      /**
       * Just calls SIG_VisualisationWidget::mousePressEvent with
       * the same QMouseEvent.
       */
      void mousePressEvent( QMouseEvent *event );

      /**
       * Just calls SIG_VisualisationWidget::mouseMoveEvent with
       * the same QMouseEvent.
       */
      void mouseMoveEvent( QMouseEvent *event );

      /**
       * The actual robot to simulate.
       */
      SIGEL_Robot::SIG_Robot const *robot;

      /**
       * The actual environment to simulate.
       */
      SIGEL_Environment::SIG_Environment const *environment;

      /**
       * The actual parameters of the simulation.
       */
      SIGEL_Simulation::SIG_SimulationParameters const *simulationParameters;

      /**
       * The actual program to control the robot in the simulation.
       */
      SIGEL_Program::SIG_Program const *program;

    private slots:

      /**
       * Lets the simulation progress about one timestep.
       *
       * Updates the widget's contents.
       * This slot is connected to the timeout signal of the
       * simulationTimer.
       */
      void slotSimulationProgress();

    signals:

      void signalSimulationProgress( QTime simulationTime );

      void signalPosition( DL_vector position );

      void signalSimulationAbort();

      void signalRecordingAllowed( bool );
    };

// }

#endif // SIGEL_SLAVEGUI_SIG_SIMULATIONVISUALISATIONWIDGET_H
