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
#include "SIGEL_SlaveGUI/SIG_SimulationWindow.h"

#include "SIGEL_SlaveGUI/SIG_SimulationWidget.h"
#include "SIGEL_SlaveGUI/SIG_SimulationVisualisationWidget.h"

#include <QStatusBar>

namespace SIGEL_SlaveGUI
{

  SIG_SimulationWindow::SIG_SimulationWindow( QWidget *parent,
					      char const *name,
					      Qt::WindowFlags f )
    : QMainWindow( parent, f )
  {
    if ( name )
      setObjectName( QString::fromUtf8( name ) );

    // Qt 2's setUsesTextLabel(false) is Qt 6's ToolButtonIconOnly.
    this->setToolButtonStyle( Qt::ToolButtonIconOnly );

    // 25x25 is what these XPMs are, measured from the files.
    this->setIconSize( QSize( 25, 25 ) );

    // Without setStatusBar the bar is an unmanaged child painted over the
    // central widget, and the setStatusTip strings below never show.
    setStatusBar( new QStatusBar( this ) );

    simulationWidget = new SIG_SimulationWidget(this, "simulationWidget");
    setCentralWidget( simulationWidget );

    simulationControlBar = new QToolBar( this );
    simulationControlBar->setObjectName( "simulationControlBar" );

    // Qt 2's QMainWindow::setDockEnabled( Left/Right/Bottom, false ) restricted
    // where a dock window could go. Qt 6 has no per-window equivalent; the
    // restriction is per toolbar, and only Top was left enabled.
    simulationControlBar->setAllowedAreas( Qt::TopToolBarArea );

    simulationControls = new SIG_SimulationControls(this, "simulationControls");

    // Qt 2's QActionGroup::addTo(w) added every member action to the widget.
    simulationControlBar->addActions( simulationControls->actions() );
    this->addToolBar( simulationControlBar );

    QObject::connect( simulationControls->playAction,
		      SIGNAL(triggered()),
		      simulationWidget->visualisationWidget,
		      SLOT(slotStartSimulation()) );

    QObject::connect( simulationControls->stopAction,
		      SIGNAL(triggered()),
		      simulationWidget,
		      SLOT(slotStopSimulation()) );

    QObject::connect( simulationControls->stepAction,
		      SIGNAL(triggered()),
		      simulationWidget->visualisationWidget,
		      SLOT(slotStepSimulation()) );

    QObject::connect( simulationControls->fForwardAction,
		      SIGNAL(triggered()),
		      simulationWidget->visualisationWidget,
		      SLOT(slotFForwardSimulation()) );

    QObject::connect( simulationWidget->visualisationWidget,
		      SIGNAL(signalSimulationAbort()),
		      simulationControls,
		      SLOT(slotPlayPressed()) );

    QObject::connect( simulationWidget->visualisationWidget,
		      SIGNAL( signalRecordingAllowed( bool ) ),
		      simulationControls,
		      SLOT( slotRecordingAllowed( bool ) ) );

    QObject::connect( simulationControls->alterMovieSettingsAction,
		      SIGNAL(triggered()),
		      simulationWidget->visualisationWidget,
		      SLOT( slotAlterMovieSettingsClicked() ) );

    QObject::connect( simulationControls->stopAction,
		      SIGNAL(triggered()),
		      this,
		      SLOT( slotStopPressed()) );

    this->resize( 1098, 900 );
    this->setMinimumSize( 780, 839 );
  };

  SIG_SimulationWindow::~SIG_SimulationWindow()
  { };

  bool SIG_SimulationWindow::event( QEvent *e )
  {
    if (e->type() == QEvent::WindowBlocked)
      simulationWidget->visualisationWidget->pauseForDialog();
    else if (e->type() == QEvent::WindowUnblocked)
      simulationWidget->visualisationWidget->resumeAfterDialog();

    return QMainWindow::event( e );
  };

  void SIG_SimulationWindow::visualizeThis(SIGEL_Robot::SIG_Robot const &robot,
					   SIGEL_Environment::SIG_Environment const &environment,
					   SIGEL_Simulation::SIG_SimulationParameters const &simulationParameters,
					   SIGEL_Program::SIG_Program const &program)
  {
    simulationWidget->visualizeThis(robot, environment, simulationParameters, program);
  };

  void SIG_SimulationWindow::slotStopPressed()
  {
    // reset the button
    simulationControls->alterMovieSettingsAction->setIcon( simulationControls->recordingDisallowedIcons );

    // reset the data that has to be resetted in the simulationVisualisationWidget
    simulationWidget->visualisationWidget->resetRecorder();
  };

}
