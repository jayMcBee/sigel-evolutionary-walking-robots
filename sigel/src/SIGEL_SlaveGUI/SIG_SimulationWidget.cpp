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
#include "SIGEL_SlaveGUI/SIG_SimulationWidget.h"

#include <QSlider>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>

#include <cmath>

#include "SIGEL_SlaveGUI/SIG_SimulationVisualisationWidget.h"

namespace SIGEL_SlaveGUI
{

  SIG_SimulationWidget::SIG_SimulationWidget( QWidget *parent,
					      char const *name,
					      Qt::WindowFlags f )
    : SIG_SimulationWidgetBase( parent, name, f )
  {
    connect( yawSlider,
	     SIGNAL(valueChanged(int)),
	     SLOT(slotSetYaw(int)) );

    connect( pitchSlider,
	     SIGNAL(valueChanged(int)),
	     SLOT(slotSetPitch(int)) );

    connect( distanceSlider,
	     SIGNAL(valueChanged(int)),
	     SLOT(slotSetDistance(int)) );

    connect( visualisationWidget,
	     SIGNAL(signalMouseRotation(int,int)),
	     SLOT(slotMouseRotation(int,int)) );

    connect( visualisationWidget,
	     SIGNAL(signalMouseZoom(int)),
	     SLOT(slotMouseZoom(int)) );

    connect( visualisationWidget,
	     SIGNAL(signalSimulationProgress(QTime)),
	     SLOT(slotSetSimulationTime(QTime)) );

    connect( visualisationWidget,
	     SIGNAL(signalPosition(DL_vector)),
	     SLOT(slotSetPosition(DL_vector)) );

    connect( traceRobotCheckBox,
	     SIGNAL(toggled(bool)),
	     SLOT(slotSetTraceRobot(bool)) );

    QObject::connect( forwardPushButton,
		      SIGNAL(pressed()),
		      visualisationWidget,
		      SLOT(slotNavigateForward()) );

    QObject::connect( backwardPushButton,
		      SIGNAL(pressed()),
		      visualisationWidget,
		      SLOT(slotNavigateBackward()) );

    QObject::connect( rightPushButton,
		      SIGNAL(pressed()),
		      visualisationWidget,
		      SLOT(slotNavigateRight()) );

    QObject::connect( leftPushButton,
		      SIGNAL(pressed()),
		      visualisationWidget,
		      SLOT(slotNavigateLeft()) );

    QObject::connect( downPushButton,
		      SIGNAL(pressed()),
		      visualisationWidget,
		      SLOT(slotNavigateDown()) );

    QObject::connect( upPushButton,
		      SIGNAL(pressed()),
		      visualisationWidget,
		      SLOT(slotNavigateUp()) );

    QObject::connect( centerPushButton,
		      SIGNAL(pressed()),
		      visualisationWidget,
		      SLOT(slotNavigateCenter()) );

    QObject::connect( frameDelaySpinBox,
		      SIGNAL(valueChanged(int)),
		      visualisationWidget,
		      SLOT(slotSetFrameDelay(int)) );

    QObject::connect( showAncorPointsCheckBox,
		      SIGNAL(stateChanged(int)),
		      visualisationWidget,
		      SLOT(setShowAncorPoints(int)) );

    QObject::connect( renderModeComboBox,
		      SIGNAL(textActivated(const QString &)),
		      visualisationWidget,
		      SLOT(setRenderMode(const QString &)) );

    QObject::connect( traceRobotCheckBox,
		      SIGNAL(toggled(bool)),
		      visualisationWidget,
		      SLOT(slotSetTraceRobot(bool)) );

    QObject::connect( ambientLightingSlider,
		      SIGNAL(valueChanged(int)),
		      visualisationWidget,
		      SLOT(setAmbientLighting(int)) );

    // Only the shaded modes draw with lighting; elsewhere the ambient light
    // has no effect.
    auto const enableAmbientLighting = [this]( QString const &mode )
      {
	bool const lit = (mode == "Flatshaded") || (mode == "Gouraudshaded");
	TextLabel1_4->setEnabled( lit );
	ambientLightingSlider->setEnabled( lit );
      };
    enableAmbientLighting( renderModeComboBox->currentText() );
    QObject::connect( renderModeComboBox, &QComboBox::textActivated, this,
		      enableAmbientLighting );

    QObject::connect( planecolorPushButton,
		      SIGNAL(clicked()),
		      visualisationWidget,
		      SLOT(choosePlaneColor()) );

    QObject::connect( showPlaneCheckBox,
		      SIGNAL(stateChanged(int)),
		      visualisationWidget,
		      SLOT(setShowPlane(int)) );

    QObject::connect( showGridCheckBox,
		      SIGNAL(stateChanged(int)),
		      visualisationWidget,
		      SLOT(setShowGrid(int)) );

    QObject::connect( showRobotPathCheckBox,
		      SIGNAL(stateChanged(int)),
		      visualisationWidget,
		      SLOT(setShowRobotPath(int)) );
  };

  SIG_SimulationWidget::~SIG_SimulationWidget()
  { };

  void SIG_SimulationWidget::slotSetYaw( int yaw )
  {
    visualisationWidget->setYaw( 360 - yaw );
  };

  void SIG_SimulationWidget::slotSetPitch( int pitch )
  {
    visualisationWidget->setPitch( pitch - 90 );
  };

  void SIG_SimulationWidget::slotSetDistance( int distance )
  {
    visualisationWidget->setDistance( static_cast<double>(distance) / 10 );
  };

  void SIG_SimulationWidget::slotMouseRotation( int deltaX, int deltaY )
  {
    int newYawValue = yawSlider->value() + deltaX;
    int newFittedYawValue = (newYawValue < 0) ? (360 - newYawValue % 360) : (newYawValue % 360);
    int newPitchValue = pitchSlider->value() + deltaY;

    yawSlider->setValue( newFittedYawValue );
    pitchSlider->setValue( newPitchValue );
  };

  void SIG_SimulationWidget::slotMouseZoom( int deltaY )
  {
    int newDistanceValue = distanceSlider->value() + deltaY;

    distanceSlider->setValue( newDistanceValue );
  };

  void SIG_SimulationWidget::slotSetSimulationTime( QTime simulationTime )
  { char txt[256];
  	QString myTime;

  	sprintf(txt, "%d hours  %d min.  %d secs.", simulationTime.hour(), simulationTime.minute(), simulationTime.second());
	myTime = txt;

	simulationTimeTextLabel->setText( myTime );
  };

  void SIG_SimulationWidget::visualizeThis(SIGEL_Robot::SIG_Robot const &robot,
					   SIGEL_Environment::SIG_Environment const &environment,
					   SIGEL_Simulation::SIG_SimulationParameters const &simulationParameters,
					   SIGEL_Program::SIG_Program const &program)
  {
    visualisationWidget->visualizeThis(robot, environment, simulationParameters, program);

    distanceSlider->setValue( qRound( visualisationWidget->getFittingDistance() * 10 ) );

    slotSetYaw( yawSlider->value() );
    slotSetPitch ( pitchSlider->value() );
    slotSetDistance( distanceSlider->value() );

    visualisationWidget->slotSetTraceRobot( traceRobotCheckBox->isChecked() );
    visualisationWidget->setRenderMode( renderModeComboBox->currentText() );
    visualisationWidget->setAmbientLighting( ambientLightingSlider->value() );

    QString programCode;
    SIGEL_Program::SIG_Program &varProgram = const_cast< SIGEL_Program::SIG_Program& >( program );
    varProgram.printToString( programCode );
    robotProgramView->setText( programCode );
  };

  void SIG_SimulationWidget::slotSetTraceRobot( bool newValue )
  {
    bool manualNavigation = !newValue;

    downPushButton->setEnabled( manualNavigation );
    upPushButton->setEnabled( manualNavigation );
    rightPushButton->setEnabled( manualNavigation );
    leftPushButton->setEnabled( manualNavigation );
    backwardPushButton->setEnabled( manualNavigation );
    forwardPushButton->setEnabled( manualNavigation );
    centerPushButton->setEnabled( manualNavigation );
  };

  void SIG_SimulationWidget::slotSetPosition( DL_vector position )
  {
    QString xPosString = QString::number( position.get( 0 ) );
    QString yPosString = QString::number( position.get( 1 ) );
    QString zPosString = QString::number( position.get( 2 ) );

    xPosTextLabel->setText( xPosString );
    yPosTextLabel->setText( yPosString );
    zPosTextLabel->setText( zPosString );
  };

  void SIG_SimulationWidget::slotStopSimulation()
  {
    visualisationWidget->slotStopSimulation();

    slotSetYaw( yawSlider->value() );
    slotSetPitch ( pitchSlider->value() );
    slotSetDistance( distanceSlider->value() );

    visualisationWidget->slotSetTraceRobot( traceRobotCheckBox->isChecked() );
    visualisationWidget->setRenderMode( renderModeComboBox->currentText() );

    visualisationWidget->setShowPlane( showPlaneCheckBox->isChecked() ? 2 : 0 );
    visualisationWidget->setShowGrid( showGridCheckBox->isChecked() ? 2 : 0 );
    visualisationWidget->setShowRobotPath( showRobotPathCheckBox->isChecked() ? 2 : 0 );

    int showAnchorPointsState;

    switch (showAncorPointsCheckBox->checkState())
      {
      case Qt::Unchecked:
	showAnchorPointsState = 0;
	break;
      case Qt::PartiallyChecked:
	showAnchorPointsState = 1;
	break;
      case Qt::Checked:
	showAnchorPointsState = 2;
	break;
      };

    visualisationWidget->setShowAncorPoints( showAnchorPointsState );
  };

}
