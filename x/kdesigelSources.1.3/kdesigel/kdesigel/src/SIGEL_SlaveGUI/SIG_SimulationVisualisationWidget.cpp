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

#include <qmessagebox.h>
#include <qspinbox.h>
#include <qlineedit.h>
#include <qpixmap.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qcolordialog.h>
#include <qwmatrix.h>

// namespace SIGEL_SlaveGUI
// {

  SIG_SimulationVisualisationWidget::SIG_SimulationVisualisationWidget( QWidget *parent,
									char const *name,
									WFlags f )
    : SIG_VisualisationWidget( parent, name, f ),
		       frameDelay(0),
		       noOfFFSteps(0),
		       traceRobot(true),
		       robot(0),
		       environment(0),
		       simulationParameters(0),
		       program(0),
		       movieWidth(320),
		       movieHeight(200),
		       movieFrequency(1),
		       movieMaxFrames(1000),
		       movieQuality(50),
		       currentFrame(0),
		       currentFrameName(0),
		       movieFilePrefix( "sigel_pic" ),
		       fileFormat( "png" ),
		       record(false),
		       useLeadingZeros( true ),
		       planeColor( 127, 127, 127 )
  {
    simulationTimer = new QTimer(this, "simulationTimer");

    connect( simulationTimer,
	     SIGNAL(timeout()),
	     SLOT(slotSimulationProgress()) );

#ifdef _WINDOWS
    char *sigelRoot = ::getenv( "SIGEL_ROOT" );
#else
    char *sigelRoot = ::getenv( "SIGEL_ROOT" );
#endif
    QString temp( sigelRoot );
    if( temp.right(1) != "/" )
      temp.append( "/" );
    movieDirectory = temp + "movie/";
  };

  SIG_SimulationVisualisationWidget::~SIG_SimulationVisualisationWidget()
  { };

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


	    if (automaticRefresh)
	      updateGL();
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
	  updateGL();
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
	  updateGL();
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
	  updateGL();
      };
  };

  void SIG_SimulationVisualisationWidget::mousePressEvent( QMouseEvent *event )
  {
    SIG_VisualisationWidget::mousePressEvent( event );
  }; 

  void SIG_SimulationVisualisationWidget::mouseMoveEvent( QMouseEvent *event )
  {
    SIG_VisualisationWidget::mouseMoveEvent( event );
  };



  void SIG_SimulationVisualisationWidget::makeTimeSteps(int noOfSteps)
  {
    if (visualisation)
      {
	SIGEL_Visualisation::SIG_SimulationVisualisation &simulationVisualisation = static_cast<SIGEL_Visualisation::SIG_SimulationVisualisation&>(*visualisation);

	try
	  {
	    simulationVisualisation.makeTimeSteps( noOfSteps );
	  }
	catch (SIGEL_Tools::SIG_Exception &e)
	  {
	    if (simulationRunning())
	      {
		slotStartSimulation();
		emit signalSimulationAbort();
	      };

	    QMessageBox::warning( this, "Simulation Exception!", e.getMessage() );
	  };

	emit signalSimulationProgress( simulationVisualisation.getSimulationTime() );

	if ( traceRobot )
	  {
	    DL_vector robotsRealOrigin = robot->initialLocation;
	    robotsRealOrigin.timesis( -1 );

	    simulationVisualisation.getRobotRotation().times( &robotsRealOrigin,
							      &simulationVisualisation.viewSettings.lookPoint );

	    DL_vector robotPosition = simulationVisualisation.getRobotPosition();

	    simulationVisualisation.viewSettings.lookPoint.plusis( &robotPosition );
	  };

	if ( record )
	  {
	    // only do the stuff every n-th frame if we have not exeeded the maximum frame number.
	    if( ((currentFrame % movieFrequency) == 0) && currentFrameName < movieMaxFrames )
	      {
		// this string will hold the fileName which has to be build...
		QString fileName = QString::number( currentFrameName );

		// check if leading zeros are wanted
		if( useLeadingZeros )
		  {
		    // O.K. leading zeros wanted
		    // compute the number of digits of the current frame
		    int currentLength = fileName.length();
		    // compute the number of digits of the maximum frames
		    int maxLength = QString::number( movieMaxFrames ).length();
		    // compute the difference between the two numbers.
		    int difference = maxLength - currentLength;
		    
		    // build the fileName
		    for( int i=1; i<=difference; i++ )
		      fileName.prepend( "0" );
		  }

		fileName.prepend( movieDirectory + movieFilePrefix );
		// append the format ending (bmp or png)
		fileName.append( "." + fileFormat );
#ifdef SIG_DEBUG
		SIGEL_Tools::SIG_IO::cerr << "Writing " << fileName << " in format " << movieWidth << " x " << movieHeight << " in quality " << movieQuality << ".\n";
#endif
		// check if the directory exists. otherwise create it.
		QDir movieDir( movieDirectory );
		if(!movieDir.exists() )
		  movieDir.mkdir( movieDirectory );

		bool renderSuccess;

		if ( fileFormat == "pov" )
		  {
		    QString includeFileName = movieFilePrefix + ".inc";

		    renderSuccess = simulationVisualisation.exportToPovray( includeFileName,
									    fileName );
		  }
		else
		{	// this has been moved to an external method;
			// the original code created a new pixmap each call w/o destroying it
			// which caused the rendering process to trash the machine pretty soon...
		  renderSuccess = callRenderPixMap(fileName);
		}

		if ( !renderSuccess )
		  {
		    // stop the recording!
		    // reset the record button!!!
		    record = false;
		    QMessageBox::warning( this, "File error", "Unable to write file " + fileName + ".\nPerhaps you don't have permission to write the file.");
		  };
		currentFrameName++;
	      } // end of if( ((currentFrame % movieFrequency) == 0) && currentFrameName < movieMaxFrames )
	    currentFrame++;
	  } // end of if(record && !automaticRefresh )
      };
  };

  bool SIG_SimulationVisualisationWidget::callRenderPixMap( QString inFName )
  {
	  bool	res;
	  int	pX=0, pY=0;
	  int	pW=-1,pH=-1;
	  double	scaleW=1.0, scaleH=1.0;
	  QPixmap	pm, resPm;
	  QWMatrix	m;

	  if(cropImage){	// speichert den Ausschnitt gegebener Größe
							// um den Mittelpunkt
							// führt keine Größenanpassung aus
		  pW = movieWidth;
		  if( (pX = (width()-movieWidth)/2) < 0){
			pX = 0;
			pW = width();
		  }
		  pH = movieHeight;
		  if( (pY = (height()-movieHeight)/2) < 0){
			  pY = 0;
			  pH = height();
		  }

	  } else if(keepRatio){	// speichere das gesamte Bild
							// passe es an die gegebene Größe an (shrink to fit)
							// behalte dabei das Seitenverhältnis bei
		  scaleW = (double) movieWidth / (double) width();
		  scaleH = (double) movieHeight/ (double) height();

	  } else {				// zooms/shrinks das Bild und schneidet einen Teil ab
		  scaleW = (double) movieWidth / (double) width();
		  scaleH = (double) movieHeight/ (double) height();
		  if( scaleW >= 1.0  ||  scaleH >= 1.0 ){			// we have to zoom in
			  if(scaleW >= scaleH){
				  scaleH = scaleW;
				  pH = (width() * movieHeight) / movieWidth;
				  pY = 0.5 * ((double) height() - ((double) movieHeight / scaleW));
			  } else {
				  scaleW = scaleH;
				  pW = (movieWidth * height()) / movieHeight;
				  pX = 0.5 * ((double) width() - ((double) movieWidth / scaleH));
			  }
		  } else if( scaleW < 1.0  &&  scaleH < 1.0 ){		// we have to shrink
			  if(scaleW >= scaleH){
				  scaleH = scaleW;
				  pH = (width() * movieHeight) / movieWidth;
				  pY = 0.5 * (height()-pH);
			  } else {
				  scaleW = scaleH;
				  pW = (movieWidth * height()) / movieHeight;
				  pX = 0.5 * (width()-pW);
			  }
		  }
	  }

	  pm	= QPixmap::grabWindow(winId(), pX, pY, pW, pH);
	  m.scale(scaleW, scaleH);
	  resPm = pm.xForm(m);
	  res	= resPm.save( inFName, fileFormat.upper(), movieQuality );

	  return true;
  }


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

    visualisation = new SIGEL_Visualisation::SIG_SimulationVisualisation( *robot,
									  *environment,
									  *simulationParameters,
									  *program );

    SIGEL_Visualisation::SIG_SimulationVisualisation &simulationVisualisation = static_cast<SIGEL_Visualisation::SIG_SimulationVisualisation&>(*visualisation);

    simulationVisualisation.setPlaneColor( planeColor );

    emit signalSimulationProgress( QTime( 0, 0 ) );

    initFloatingTextWidgets();

    noOfFFSteps = static_cast<int>(5 / simulationParameters->getStepSize());

    resizeGL( width(), height() );

    slotSetTraceRobot( true );
    slotNavigateCenter();
    updateGL();
  };

void SIG_SimulationVisualisationWidget::resetRecorder()
{
  record = false;
  currentFrame = 0;
  currentFrameName = 0;
};

  bool SIG_SimulationVisualisationWidget::simulationRunning()
  {
    return simulationTimer->isActive();
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
	simulationTimer->start( frameDelay );
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
	updateGL();
      };
  };

  void SIG_SimulationVisualisationWidget::slotFForwardSimulation()
  {
    if (!simulationRunning())
      {
	makeTimeSteps( noOfFFSteps );
	updateGL();
      };
  };

  void SIG_SimulationVisualisationWidget::slotSetFrameDelay(int fframeDelay)
  {
    frameDelay = fframeDelay;

    if (simulationTimer->isActive())
      simulationTimer->changeInterval( frameDelay );
  };

  void SIG_SimulationVisualisationWidget::slotSimulationProgress()
  {
    makeTimeSteps(1);
    updateGL();
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

	double radViewDirectionAngle = (viewDirectionAngle / 360) * 2 * pi;

#ifdef _WINDOWS
	double newXPos = actXPos + ( ::sin( radViewDirectionAngle ) * distance );
	double newZPos = actZPos + ( ::cos( radViewDirectionAngle ) * distance );
#else
	double newXPos = actXPos + ( std::sin( radViewDirectionAngle ) * distance );
	double newZPos = actZPos + ( std::cos( radViewDirectionAngle ) * distance );
#endif	

	visualisation->viewSettings.lookPoint.set( 0 , newXPos );
	visualisation->viewSettings.lookPoint.set( 2 , newZPos );

	if (automaticRefresh)
	  updateGL();
      };
  };

  void SIG_SimulationVisualisationWidget::slotNavigateBackward()
  {
    if (visualisation)
      {
	double const distance = 0.1;

	double actXPos = visualisation->viewSettings.lookPoint.get( 0 );
	double actZPos = visualisation->viewSettings.lookPoint.get( 2 );

	double radViewDirectionAngle = (yaw / 360) * 2 * pi;

#ifdef _WINDOWS
	double newXPos = actXPos + ( ::sin( radViewDirectionAngle ) * distance );
	double newZPos = actZPos + ( ::cos( radViewDirectionAngle ) * distance );
#else
	double newXPos = actXPos + ( std::sin( radViewDirectionAngle ) * distance );
	double newZPos = actZPos + ( std::cos( radViewDirectionAngle ) * distance );
#endif	

	visualisation->viewSettings.lookPoint.set( 0 , newXPos );
	visualisation->viewSettings.lookPoint.set( 2 , newZPos );

	if (automaticRefresh)
	  updateGL();
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

	double radViewDirectionAngle = (viewDirectionAngle / 360) * 2 * pi;

#ifdef _WINDOWS
	double newXPos = actXPos + ( ::sin( radViewDirectionAngle ) * distance );
	double newZPos = actZPos + ( ::cos( radViewDirectionAngle ) * distance );
#else
	double newXPos = actXPos + ( std::sin( radViewDirectionAngle ) * distance );
	double newZPos = actZPos + ( std::cos( radViewDirectionAngle ) * distance );
#endif	

	visualisation->viewSettings.lookPoint.set( 0 , newXPos );
	visualisation->viewSettings.lookPoint.set( 2 , newZPos );

	if (automaticRefresh)
	  updateGL();
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

	double radViewDirectionAngle = (viewDirectionAngle / 360) * 2 * pi;

#ifdef _WINDOWS
	double newXPos = actXPos + ( ::sin( radViewDirectionAngle ) * distance );
	double newZPos = actZPos + ( ::cos( radViewDirectionAngle ) * distance );
#else
	double newXPos = actXPos + ( std::sin( radViewDirectionAngle ) * distance );
	double newZPos = actZPos + ( std::cos( radViewDirectionAngle ) * distance );
#endif	

	visualisation->viewSettings.lookPoint.set( 0 , newXPos );
	visualisation->viewSettings.lookPoint.set( 2 , newZPos );

	if (automaticRefresh)
	  updateGL();
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
	  updateGL();
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
	  updateGL();
      };
  };

  void SIG_SimulationVisualisationWidget::slotNavigateCenter()
  {
    if (visualisation)
      {
	SIGEL_Visualisation::SIG_SimulationVisualisation *simulationVisualisation =
	  static_cast< SIGEL_Visualisation::SIG_SimulationVisualisation* >( visualisation );

	DL_vector robotsRealOrigin = robot->initialLocation;
	robotsRealOrigin.timesis( -1 );

	simulationVisualisation->getRobotRotation().times( &robotsRealOrigin,
							   &simulationVisualisation->viewSettings.lookPoint );

	DL_vector robotPosition = simulationVisualisation->getRobotPosition();

	simulationVisualisation->viewSettings.lookPoint.plusis( &robotPosition );

	if (automaticRefresh)
	  updateGL();
      };
  };

  void SIG_SimulationVisualisationWidget::slotRecordClicked()
  {
    record = !record;
  };

  void SIG_SimulationVisualisationWidget::slotAlterMovieSettingsClicked()
  {
    SIGEL_SlaveGUI::SIG_MovieSettingsDialog movieSettingsDialog( width(), height(), this, "movieSettingsDialog", true );
    movieSettingsDialog.spinboxWidth->setValue( movieWidth );
    movieSettingsDialog.spinboxHeight->setValue( movieHeight );
    movieSettingsDialog.spinboxFrequency->setValue( movieFrequency );
    movieSettingsDialog.lineeditDirectory->setText( movieDirectory );
    movieSettingsDialog.lineeditFilePrefix->setText( movieFilePrefix );
    movieSettingsDialog.spinboxMaxFrames->setValue( movieMaxFrames );
    if( fileFormat == "bmp" )
      movieSettingsDialog.comboboxFormat->setCurrentItem(0);
    if( fileFormat == "png" )
      movieSettingsDialog.comboboxFormat->setCurrentItem(1);
    if( fileFormat == "ppm" )
      movieSettingsDialog.comboboxFormat->setCurrentItem(2);
    if( fileFormat == "xbm" )
      movieSettingsDialog.comboboxFormat->setCurrentItem(3);
    if( fileFormat == "xpm" )
      movieSettingsDialog.comboboxFormat->setCurrentItem(4);
    if( fileFormat == "pov" )
      movieSettingsDialog.comboboxFormat->setCurrentItem(5);
    movieSettingsDialog.spinboxQuality->setValue( movieQuality );
    movieSettingsDialog.checkboxUseLeadingZeros->setChecked( useLeadingZeros );
    movieSettingsDialog.checkboxEnableMovie->setChecked( record );

    switch( movieSettingsDialog.exec() )
      {
      case QDialog::Accepted:
      keepRatio = movieSettingsDialog.checkboxKeepAspectRatio->isChecked();
      cropImage = movieSettingsDialog.checkboxCropImage->isChecked();
	movieWidth = movieSettingsDialog.spinboxWidth->value();
	movieHeight = movieSettingsDialog.spinboxHeight->value();
	movieFrequency = movieSettingsDialog.spinboxFrequency->value();
	movieDirectory = movieSettingsDialog.lineeditDirectory->text();
	if( movieDirectory.right(1) != "/" )
	  movieDirectory.append( "/" );
	movieFilePrefix = movieSettingsDialog.lineeditFilePrefix->text();
	fileFormat = movieSettingsDialog.comboboxFormat->currentText().lower();
	movieMaxFrames = movieSettingsDialog.spinboxMaxFrames->value();
	movieQuality = movieSettingsDialog.spinboxQuality->value();
	useLeadingZeros = movieSettingsDialog.checkboxUseLeadingZeros->isChecked();
	record = movieSettingsDialog.checkboxEnableMovie->isChecked();

	if (record && (fileFormat == "pov") )
	  {
	    SIGEL_Visualisation::SIG_SimulationVisualisation &simulationVisualisation =
	      static_cast< SIGEL_Visualisation::SIG_SimulationVisualisation& >( *visualisation );

	    QString fileName = movieDirectory + movieFilePrefix + ".inc";

	    QDir movieDir( movieDirectory );
	    if(!movieDir.exists() )
	      movieDir.mkdir( movieDirectory );

	    double aspectRatio = double( movieWidth ) / double ( movieHeight );

	    if ( !simulationVisualisation.createPovrayIncludeFile( fileName, aspectRatio ) )
	      {
		record = false;
		QMessageBox::warning( this, "File error", "Unable to write file " + fileName + ".\nPerhaps you don't have permission to write the file.");
	      };
	  };

	emit signalRecordingAllowed( record );
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
