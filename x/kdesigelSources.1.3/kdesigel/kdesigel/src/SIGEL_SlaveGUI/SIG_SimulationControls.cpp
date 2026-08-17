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
#include "SIGEL_SlaveGUI/SIG_SimulationControls.h"

#include <qapplication.h>

#include <cstdlib>

namespace SIGEL_SlaveGUI
{

  SIG_SimulationControls::SIG_SimulationControls(QMainWindow *mainWindow, QString name)
    : QActionGroup( mainWindow ),
      mainWindow(mainWindow),
      simulationRunning( false )
  {
#ifdef _WINDOWS
    char *sigelRootCString = ::getenv( "SIGEL_ROOT" );
#else
    char *sigelRootCString = std::getenv( "SIGEL_ROOT" );
#endif

    QString sigelRootString( sigelRootCString );

    stopAction = new QAction( this, "stopAction" );
    playAction = new QAction( this, "playAction" );
    stepAction = new QAction( this, "stepAction" );
    fForwardAction = new QAction( this, "fForwardAction" );
    // recordAction = new QAction( this, "recordAction" );
    alterMovieSettingsAction = new QAction( this, "alterMovieSettingsAction" );
    quitAction = new QAction( this, "quitAction" );

    stopIcons.setPixmap( sigelRootString + "/pixmaps/stopButton.xpm", QIconSet::Large );
    playIcons.setPixmap( sigelRootString + "/pixmaps/startButton.xpm", QIconSet::Large );
    stepIcons.setPixmap( sigelRootString + "/pixmaps/stepButton.xpm", QIconSet::Large );
    pauseIcons.setPixmap( sigelRootString + "/pixmaps/pauseButton.xpm", QIconSet::Large );
    fForwardIcons.setPixmap( sigelRootString + "/pixmaps/ffButton.xpm", QIconSet::Large );
    // recordIcons.setPixmap( sigelRootString + "/pixmaps/videoallow.xpm", QIconSet::Large );
    recordingAllowedIcons.setPixmap( sigelRootString + "/pixmaps/videoallow.xpm", QIconSet::Large );
    recordingDisallowedIcons.setPixmap( sigelRootString + "/pixmaps/videodisallow.xpm", QIconSet::Large ); 
    quitIcons.setPixmap( sigelRootString + "/pixmaps/quitApplicationSmall.xpm", QIconSet::Large );

    stopAction->setIconSet( stopIcons );
    playAction->setIconSet( playIcons );
    stepAction->setIconSet( stepIcons );
    fForwardAction->setIconSet( fForwardIcons );
    // recordAction->setIconSet( recordIcons );
    alterMovieSettingsAction->setIconSet( recordingDisallowedIcons );
    quitAction->setIconSet( quitIcons );

    stopAction->setStatusTip( "Stops and restarts the simulation." );
    playAction->setStatusTip( "Starts/pauses the simulation." );
    stepAction->setStatusTip( "Lets the simulation progress about one timestep." );
    fForwardAction->setStatusTip( "Lets the simulation progress about 5 seconds." );
    // recordAction->setStatusTip( "Toggle the recording of the simulation." );
    alterMovieSettingsAction->setStatusTip( "Alter the movie settings." );
    quitAction->setStatusTip( "Closes the simulation visualization." );

    connect( playAction,
	     SIGNAL(activated()),
	     SLOT(slotPlayPressed()) );

    connect( stopAction,
	     SIGNAL(activated()),
	     SLOT(slotStopPressed()) );

    QObject::connect( quitAction,
		      SIGNAL(activated()),
		      qApp,
		      SLOT(quit()) );

  };

  SIG_SimulationControls::~SIG_SimulationControls()
  { };

  void SIG_SimulationControls::slotPlayPressed()
  {
    simulationRunning = !simulationRunning;

    stepAction->setEnabled( !simulationRunning );
    fForwardAction->setEnabled( !simulationRunning );

    if (simulationRunning)
      playAction->setIconSet( pauseIcons );
    else
      playAction->setIconSet( playIcons );
  };

  void SIG_SimulationControls::slotStopPressed()
  {
    simulationRunning = false;

    stepAction->setEnabled( true );
    fForwardAction->setEnabled( true );

    playAction->setIconSet( playIcons );
  };

  void SIG_SimulationControls::slotRecordingAllowed( bool isAllowed )
  {
    if( isAllowed )
      alterMovieSettingsAction->setIconSet( recordingAllowedIcons );
    else
      alterMovieSettingsAction->setIconSet( recordingDisallowedIcons );
  };
}
