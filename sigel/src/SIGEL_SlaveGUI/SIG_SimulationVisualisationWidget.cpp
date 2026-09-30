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
#include "SIGEL_SlaveGUI/SIG_SimulationVisualisationWidget.h"

#include "SIGEL_Visualisation/SIG_SimulationVisualisation.h"
#include "SIGEL_SlaveGUI/SIG_MovieSettingsDialog.h"
#include "SIGEL_Tools/SIG_Exception.h"

#include <QMessageBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QPixmap>
#include <QComboBox>
#include <QCheckBox>
#include <QColorDialog>

#include <QtMath>
#include <cmath>

// namespace SIGEL_SlaveGUI
// {

  SIG_SimulationVisualisationWidget::SIG_SimulationVisualisationWidget( QWidget *parent,
									char const *name,
									Qt::WindowFlags f )
    : SIG_VisualisationWidget( parent, name, f ),
      frameDelay(0),
      pausedForDialog(false),
      frameShown(true),
      noOfFFSteps(0),
      traceRobot(true),
      planeColor( 127, 127, 127 ),
      gridColor( 80, 80, 80 ),
      movieEncoder( this ),
      robot(nullptr),
      environment(nullptr),
      simulationParameters(nullptr),
      program(nullptr),
      startRobotCentre( 0.0, 0.0, 0.0 )
  {
    simulationTimer = new QTimer( this );
    simulationTimer->setObjectName( "simulationTimer" );

    connect( simulationTimer,
	     SIGNAL(timeout()),
	     SLOT(slotSimulationProgress()) );

    // Play steps once per frame on screen; see slotSimulationProgress.
    connect( this,
	     SIGNAL(frameSwapped()),
	     SLOT(slotFrameShown()) );

    char *sigelRoot = ::getenv( "SIGEL_ROOT" );
    QString temp( sigelRoot );
    if( temp.right(1) != "/" )
      temp.append( "/" );
    SIGEL_SlaveGUI::SIG_MovieSettings movieSettings;
    movieSettings.directory = temp + "movie/";
    movieRecorder.setSettings( movieSettings );
  };

  void SIG_SimulationVisualisationWidget::setShowAncorPoints( int state )
  {
    if (visualisation)
      {
	SIGEL_Visualisation::SIG_SimulationVisualisation *simulationVisualisation =
	  static_cast< SIGEL_Visualisation::SIG_SimulationVisualisation* >( visualisation );

	if (state > 0)
	  simulationVisualisation->setPointsVisible( true );
	else
	  simulationVisualisation->setPointsVisible( false );

	SIG_VisualisationWidget::setShowAncorPoints( state );
      };
  };

  void SIG_SimulationVisualisationWidget::choosePlaneColor()
  {
    if (visualisation)
      {
	SIGEL_Visualisation::SIG_SimulationVisualisation &simulationVisualisation = static_cast<SIGEL_Visualisation::SIG_SimulationVisualisation&>(*visualisation);

	QColor newPlaneColor = QColorDialog::getColor( planeColor, this );

	if (newPlaneColor.isValid())
	  {
	    planeColor = newPlaneColor;
	    simulationVisualisation.setPlaneColor( planeColor );
	    emit signalPlaneColorChanged( planeColor );

	    if (automaticRefresh)
	      update();
	  };
      };
  };

  void SIG_SimulationVisualisationWidget::chooseGridColor()
  {
    if (visualisation)
      {
	SIGEL_Visualisation::SIG_SimulationVisualisation &simulationVisualisation = static_cast<SIGEL_Visualisation::SIG_SimulationVisualisation&>(*visualisation);

	QColor newGridColor = QColorDialog::getColor( gridColor, this );

	if (newGridColor.isValid())
	  {
	    gridColor = newGridColor;
	    simulationVisualisation.setGridColor( gridColor );
	    emit signalGridColorChanged( gridColor );

	    if (automaticRefresh)
	      update();
	  };
      };
  };

  void SIG_SimulationVisualisationWidget::setShowPlane( int state )
  {
    if (visualisation)
      {
	SIGEL_Visualisation::SIG_SimulationVisualisation &simulationVisualisation =
	  static_cast< SIGEL_Visualisation::SIG_SimulationVisualisation& >( *visualisation );

	switch (state)
	  {
	  case 0:
	    simulationVisualisation.setShowPlane( false );
	    break;
	  case 2:
	    simulationVisualisation.setShowPlane( true );
	    break;
	  };

	if (automaticRefresh)
	  update();
      };
  };

  void SIG_SimulationVisualisationWidget::setShowGrid( int state )
  {
    if (visualisation)
      {
	SIGEL_Visualisation::SIG_SimulationVisualisation &simulationVisualisation =
	  static_cast< SIGEL_Visualisation::SIG_SimulationVisualisation& >( *visualisation );

	switch (state)
	  {
	  case 0:
	    simulationVisualisation.setShowGrid( false );
	    break;
	  case 2:
	    simulationVisualisation.setShowGrid( true );
	    break;
	  };

	if (automaticRefresh)
	  update();
      };
  };

  void SIG_SimulationVisualisationWidget::setShowRobotPath( int state )
  {
    if (visualisation)
      {
	SIGEL_Visualisation::SIG_SimulationVisualisation &simulationVisualisation =
	  static_cast< SIGEL_Visualisation::SIG_SimulationVisualisation& >( *visualisation );

	switch (state)
	  {
	  case 0:
	    simulationVisualisation.setShowRobotPath( false );
	    break;
	  case 2:
	    simulationVisualisation.setShowRobotPath( true );
	    break;
	  };

	if (automaticRefresh)
	  update();
      };
  };

  void SIG_SimulationVisualisationWidget::setShowShadows( bool show )
  {
    if (visualisation)
      {
	static_cast< SIGEL_Visualisation::SIG_SimulationVisualisation* >( visualisation )->setShowShadows( show );

	if (automaticRefresh)
	  update();
      };
  };



	void SIG_SimulationVisualisationWidget::makeTimeSteps( int noOfSteps )
	{
		if ( visualisation )
		{
			SIGEL_Visualisation::SIG_SimulationVisualisation &simulationVisualisation =
				static_cast< SIGEL_Visualisation::SIG_SimulationVisualisation& >( *visualisation );

			try
			{
				simulationVisualisation.makeTimeSteps( noOfSteps );
			}
			catch ( SIGEL_Tools::SIG_Exception &e )
			{
				if ( simulationRunning() )
				{
					slotStartSimulation();
					emit signalSimulationAbort();
				}

				QMessageBox::warning( this, "Simulation Exception", e.getMessage() );
			}

			emit signalSimulationProgress( simulationVisualisation.getCurrentSimulationSeconds() );

			if ( traceRobot )
			{
				SIG_Vector robotCentre = simulationVisualisation.getRobotCentre();
				simulationVisualisation.viewSettings.lookPoint.assign( &robotCentre );
			}

			double seconds = simulationVisualisation.getCurrentSimulationSeconds();

			if ( movieRecorder.needsToRecordFrameAt( seconds ) )
				recordFrame( simulationVisualisation, seconds );
		}
	};

	void SIG_SimulationVisualisationWidget::recordFrame( SIGEL_Visualisation::SIG_SimulationVisualisation &simulationVisualisation, double seconds )
	{
		bool written;

		if ( movieRecorder.getSettings().format == "pov" )
			written = movieRecorder.writePovray( simulationVisualisation, seconds );
		else
		{
			SIG_Vector robotCentre = simulationVisualisation.getRobotCentre();
			SIG_Vector fromStart = robotCentre;
			fromStart.minusis( &startRobotCentre );

			written = movieRecorder.writeImage( grabFramebuffer(), seconds, fromStart.norm(), robotCentre.get( 1 ) );
		}

		if ( !written )
		{
			// stop the recording, and show it on the movie button
			movieRecorder.stopRecording();
			emit signalRecordingAllowed( false );
			QMessageBox::warning( this, "File Error",
				"Unable to write file " + movieRecorder.getLastFileName() + ".\nPerhaps you don't have permission to write the file." );
		}
		else if ( movieRecorder.getFramesRecorded() >= movieRecorder.getSettings().maxFrames )
		{
			movieRecorder.stopRecording();
			emit signalRecordingAllowed( false );
			reportAndEncodeRecording();
		}
	};

  void SIG_SimulationVisualisationWidget::visualizeThis(SIGEL_Robot::SIG_Robot const &rrobot,
							SIGEL_Environment::SIG_Environment const &eenvironment,
							SIGEL_Simulation::SIG_SimulationParameters const &ssimulationParameters,
							SIGEL_Program::SIG_Program const &pprogram)
  {
    robot = &rrobot;
    environment = &eenvironment;
    simulationParameters = &ssimulationParameters;
    program = &pprogram;

    makeCurrent();

    delete visualisation;
    // Null first: the constructor below can throw, and the slots test for null.
    visualisation = nullptr;

    visualisation = new SIGEL_Visualisation::SIG_SimulationVisualisation( *robot,
									  *environment,
									  *simulationParameters,
									  *program );

    SIGEL_Visualisation::SIG_SimulationVisualisation &simulationVisualisation = static_cast<SIGEL_Visualisation::SIG_SimulationVisualisation&>(*visualisation);

    simulationVisualisation.setPlaneColor( planeColor );
    simulationVisualisation.setGridColor( gridColor );
    startRobotCentre = simulationVisualisation.getRobotCentre();

    emit signalSimulationProgress( 0.0 );

    initFloatingTextWidgets();

    noOfFFSteps = static_cast<int>(5 / simulationParameters->getStepSize());

    resizeGL( width(), height() );

    slotSetTraceRobot( true );
    slotNavigateCenter();
    update();
  };

void SIG_SimulationVisualisationWidget::resetRecorder()
{
  if ( movieRecorder.isRecording() )
    reportAndEncodeRecording();

  movieRecorder.reset();
};

void SIG_SimulationVisualisationWidget::reportAndEncodeRecording()
{
  movieEncoder.encode( movieRecorder.getSettings(), movieRecorder.getFramesRecorded() );
};

  void SIG_SimulationVisualisationWidget::slotStartSimulation()
  {
    if (simulationRunning())
      {
	automaticRefresh = true;
	simulationTimer->stop();
      }
    else
      {
	automaticRefresh = false;
	frameShown = true;
	simulationTimer->start( qMax( frameDelay, 1 ) );
      };
  };

  void SIG_SimulationVisualisationWidget::pauseForDialog()
  {
    if (simulationRunning())
      {
	simulationTimer->stop();
	pausedForDialog = true;
      };
  };

  void SIG_SimulationVisualisationWidget::resumeAfterDialog()
  {
    if (pausedForDialog)
      {
	pausedForDialog = false;
	simulationTimer->start( qMax( frameDelay, 1 ) );
      };
  };

  void SIG_SimulationVisualisationWidget::slotStopSimulation()
  {
    if (simulationRunning())
      {
	slotStartSimulation();
      };

    visualizeThis( *robot,
		   *environment,
		   *simulationParameters,
		   *program );
  };

  void SIG_SimulationVisualisationWidget::slotStepSimulation()
  {
    if (!simulationRunning())
      {
	makeTimeSteps(1);
	update();
      };
  };

  void SIG_SimulationVisualisationWidget::slotFForwardSimulation()
  {
    if (!simulationRunning())
      {
	makeTimeSteps( noOfFFSteps );
	update();
      };
  };

  void SIG_SimulationVisualisationWidget::slotSetFrameDelay(int fframeDelay)
  {
    frameDelay = fframeDelay;

    if (simulationTimer->isActive())
      simulationTimer->setInterval( qMax( frameDelay, 1 ) );
  };

  void SIG_SimulationVisualisationWidget::slotSimulationProgress()
  {
    // Wait until the last step is on screen; without a GL context no frame
    // comes, so don't wait.
    if (!frameShown && isValid())
      return;

    frameShown = false;
    makeTimeSteps(1);
    update();
  };

  void SIG_SimulationVisualisationWidget::slotFrameShown()
  {
    frameShown = true;
  };

  void SIG_SimulationVisualisationWidget::slotSetTraceRobot( bool newValue )
  {
    traceRobot = newValue;

    if (traceRobot)
      slotNavigateCenter();
  };

  void SIG_SimulationVisualisationWidget::slotNavigateForward()
  {
    if (visualisation)
      {
	// distance regulates how much a click in this control moves us in 3d space
	double const distance = 0.1;

	double actXPos = visualisation->viewSettings.lookPoint.get( 0 );
	double actZPos = visualisation->viewSettings.lookPoint.get( 2 );

	double viewDirectionAngle = yaw + 180;
	if (viewDirectionAngle > 360)
	  viewDirectionAngle -= 360;

	double radViewDirectionAngle = qDegreesToRadians( viewDirectionAngle );

	double newXPos = actXPos + ( std::sin( radViewDirectionAngle ) * distance );
	double newZPos = actZPos + ( std::cos( radViewDirectionAngle ) * distance );

	visualisation->viewSettings.lookPoint.set( 0 , newXPos );
	visualisation->viewSettings.lookPoint.set( 2 , newZPos );

	if (automaticRefresh)
	  update();
      };
  };

  void SIG_SimulationVisualisationWidget::slotNavigateBackward()
  {
    if (visualisation)
      {
	double const distance = 0.1;

	double actXPos = visualisation->viewSettings.lookPoint.get( 0 );
	double actZPos = visualisation->viewSettings.lookPoint.get( 2 );

	double radViewDirectionAngle = qDegreesToRadians( yaw );

	double newXPos = actXPos + ( std::sin( radViewDirectionAngle ) * distance );
	double newZPos = actZPos + ( std::cos( radViewDirectionAngle ) * distance );

	visualisation->viewSettings.lookPoint.set( 0 , newXPos );
	visualisation->viewSettings.lookPoint.set( 2 , newZPos );

	if (automaticRefresh)
	  update();
      };
  };

  void SIG_SimulationVisualisationWidget::slotNavigateRight()
  {
    if (visualisation)
      {
	double const distance = 0.1;

	double actXPos = visualisation->viewSettings.lookPoint.get( 0 );

	double actZPos = visualisation->viewSettings.lookPoint.get( 2 );

	double viewDirectionAngle = yaw + 90;
	if (viewDirectionAngle > 360)
	  viewDirectionAngle -= 360;

	double radViewDirectionAngle = qDegreesToRadians( viewDirectionAngle );

	double newXPos = actXPos + ( std::sin( radViewDirectionAngle ) * distance );
	double newZPos = actZPos + ( std::cos( radViewDirectionAngle ) * distance );

	visualisation->viewSettings.lookPoint.set( 0 , newXPos );
	visualisation->viewSettings.lookPoint.set( 2 , newZPos );

	if (automaticRefresh)
	  update();
      };
  };

  void SIG_SimulationVisualisationWidget::slotNavigateLeft()
  {
    if (visualisation)
      {
	double const distance = 0.1;

	double actXPos = visualisation->viewSettings.lookPoint.get( 0 );
	double actZPos = visualisation->viewSettings.lookPoint.get( 2 );

	double viewDirectionAngle = yaw - 90;
	if (viewDirectionAngle < 360)
	  viewDirectionAngle += 360;

	double radViewDirectionAngle = qDegreesToRadians( viewDirectionAngle );

	double newXPos = actXPos + ( std::sin( radViewDirectionAngle ) * distance );
	double newZPos = actZPos + ( std::cos( radViewDirectionAngle ) * distance );

	visualisation->viewSettings.lookPoint.set( 0 , newXPos );
	visualisation->viewSettings.lookPoint.set( 2 , newZPos );

	if (automaticRefresh)
	  update();
      };
  };

  void SIG_SimulationVisualisationWidget::slotNavigateDown()
  {
    if (visualisation)
      {
	double const distance = 0.1;

	double actHeight = visualisation->viewSettings.lookPoint.get( 1 );

	visualisation->viewSettings.lookPoint.set( 1, actHeight - distance );

	if (automaticRefresh)
	  update();
      };
  };

  void SIG_SimulationVisualisationWidget::slotNavigateUp()
  {
    if (visualisation)
      {
	double const distance = 0.1;

	double actHeight = visualisation->viewSettings.lookPoint.get( 1 );

	visualisation->viewSettings.lookPoint.set( 1, actHeight + distance );

	if (automaticRefresh)
	  update();
      };
  };

  bool SIG_SimulationVisualisationWidget::canShowShadows() const
  {
    return visualisation
      && static_cast< SIGEL_Visualisation::SIG_SimulationVisualisation const* >( visualisation )->canShowShadows();
  };

  double SIG_SimulationVisualisationWidget::getFittingDistance() const
  {
    if (!visualisation)
      return distance;

    SIGEL_Visualisation::SIG_SimulationVisualisation const *simulationVisualisation =
      static_cast< SIGEL_Visualisation::SIG_SimulationVisualisation const* >( visualisation );

    // The robot's enclosing sphere just fits the view. The narrower of the
    // two view angles decides, so the robot fits both ways.
    double const halfHeightAngle = qDegreesToRadians( SIGEL_Visualisation::SIG_Visualisation::fieldOfView / 2 );
    double halfAngle = halfHeightAngle;
    double const aspectRatio = visualisation->viewSettings.aspectRatio;
    if (aspectRatio < 1)
      halfAngle = std::atan( std::tan( halfHeightAngle ) * aspectRatio );

    return simulationVisualisation->getRobotRadius() / std::sin( halfAngle );
  };

  void SIG_SimulationVisualisationWidget::slotNavigateCenter()
  {
    if (visualisation)
      {
	SIGEL_Visualisation::SIG_SimulationVisualisation *simulationVisualisation =
	  static_cast< SIGEL_Visualisation::SIG_SimulationVisualisation* >( visualisation );

	SIG_Vector robotCentre = simulationVisualisation->getRobotCentre();
	simulationVisualisation->viewSettings.lookPoint.assign( &robotCentre );

	if (automaticRefresh)
	  update();
      };
  };

  void SIG_SimulationVisualisationWidget::slotAlterMovieSettingsClicked()
  {
    if ( !visualisation )
      return;

    SIGEL_Visualisation::SIG_SimulationVisualisation &simulationVisualisation =
      static_cast< SIGEL_Visualisation::SIG_SimulationVisualisation& >( *visualisation );

    SIGEL_SlaveGUI::SIG_MovieSettingsDialog movieSettingsDialog( this, simulationParameters->getStepSize(), this, "movieSettingsDialog", true );
    movieSettingsDialog.setSettings( movieRecorder.getSettings() );
    movieSettingsDialog.checkboxEnableMovie->setChecked( movieRecorder.isRecording() );

    switch( movieSettingsDialog.exec() )
      {
      case QDialog::Accepted:
	if ( movieRecorder.isRecording() && !movieSettingsDialog.checkboxEnableMovie->isChecked() )
	  reportAndEncodeRecording();

	{
	  SIGEL_SlaveGUI::SIG_MovieSettings newSettings = movieSettingsDialog.settings();
	  bool restartTiming = !movieRecorder.isRecording()
	    || newSettings.frameRate != movieRecorder.getSettings().frameRate;

	  movieRecorder.setSettings( newSettings );

	  if ( !movieSettingsDialog.checkboxEnableMovie->isChecked() )
	    movieRecorder.stopRecording();
	  else if ( restartTiming )
	    movieRecorder.startRecordingAt( simulationVisualisation.getCurrentSimulationSeconds() );
	}

	if ( movieRecorder.isRecording() && (movieRecorder.getSettings().format == "pov") )
	  {
	    if ( !movieRecorder.createPovrayIncludeFile( simulationVisualisation ) )
	      {
		movieRecorder.stopRecording();
		QMessageBox::warning( this, "File Error", "Unable to write file " + movieRecorder.getLastFileName() + ".\nPerhaps you don't have permission to write the file.");
	      };
	  };

	emit signalRecordingAllowed( movieRecorder.isRecording() );
	break;
      }
  };

  void SIG_SimulationVisualisationWidget::paintGL()
  {
    SIG_VisualisationWidget::paintGL();

    if (visualisation)
      {
	emit signalPosition( visualisation->viewSettings.lookPoint );
      };
  };

// }
