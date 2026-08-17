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

#include <qstatusbar.h>

namespace SIGEL_SlaveGUI
{

  SIG_SimulationWindow::SIG_SimulationWindow( QWidget *parent,
					      char const *name,
					      WFlags f )
    : QMainWindow( parent,
		   name,
		   f )
  {
    this->setUsesTextLabel( false );
    this->setDockEnabled( Left, false );
    this->setDockEnabled( Right, false );
    this->setDockEnabled( Bottom, false );

    this->setUsesBigPixmaps( true );

    new QStatusBar( this );

    simulationWidget = new SIG_SimulationWidget(this, "simulationWidget");
    setCentralWidget( simulationWidget );

    simulationControlBar = new QToolBar( this, "simulationControlBar" );

    simulationControls = new SIG_SimulationControls(this, "simulationControls");

    simulationControls->addTo( simulationControlBar );
    this->addToolBar( simulationControlBar );

    QObject::connect( simulationControls->playAction,
		      SIGNAL(activated()),
		      simulationWidget->visualisationWidget,
		      SLOT(slotStartSimulation()) );

    QObject::connect( simulationControls->stopAction,
		      SIGNAL(activated()),
		      simulationWidget,
		      SLOT(slotStopSimulation()) );

    QObject::connect( simulationControls->stepAction,
		      SIGNAL(activated()),
		      simulationWidget->visualisationWidget,
		      SLOT(slotStepSimulation()) );

    QObject::connect( simulationControls->fForwardAction,
		      SIGNAL(activated()),
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
		      SIGNAL( activated() ),
		      simulationWidget->visualisationWidget,
		      SLOT( slotAlterMovieSettingsClicked() ) );

    QObject::connect( simulationControls->stopAction,
		      SIGNAL(activated()),
		      this,
		      SLOT( slotStopPressed()) );

    this->resize( 780, 810 );
    this->setMinimumSize( 780, 810 );
  };

  SIG_SimulationWindow::~SIG_SimulationWindow()
  { };

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
    simulationControls->alterMovieSettingsAction->setIconSet( simulationControls->recordingDisallowedIcons );

    // reset the data that has to be resetted in the simulationVisualisationWidget
    simulationWidget->visualisationWidget->resetRecorder();
  };

}
