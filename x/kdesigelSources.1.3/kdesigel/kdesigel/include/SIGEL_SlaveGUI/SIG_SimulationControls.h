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
#ifndef SIGEL_SLAVEGUI_SIG_SIMULATIONCONTROLS_H
#define SIGEL_SLAVEGUI_SIG_SIMULATIONCONTROLS_H

#include <qaction.h>
#include <qiconset.h>
#include <qmainwindow.h>
#include <qstring.h>

namespace SIGEL_SlaveGUI
{

  /**
   * This class encapsulates the actions involved in controlling the
   * visualisation's simulation.
   *
   * Because it inherits the class QActiongroup one can use simply the
   * methods addTo and removeFrom to add respectively remove
   * all controls with one command.
   * The constructor automatically creates the QActions that make up
   * the controls and adds them to this QActionGroup.
   */
  class SIG_SimulationControls : public QActionGroup
    {

      Q_OBJECT

    public:

      /**
       * The constructor of the SIG_SimulationControls.
       *
       * Creates QAction objects for the stop, play, step and
       * fast forward actions and assigns them appropriate texts
       * and iconsets.
       *
       * @param mainWindow The QMainWindow into which the QActions contained
       *                   in this class should be inserted.
       * @param name The name of this SIG_SimulationControls.
       */
      SIG_SimulationControls(QMainWindow *mainWindow,
			     QString name = QString());

      /**
       * The destructor of the SIG_SimulationControls.
       */
      virtual ~SIG_SimulationControls();

      /**
       * The surrounding QMainWindow that contains the toolbar and menubar
       * where this QActiongroup QActions should be inserted.
       */
      QMainWindow *mainWindow;

      /**
       * The stop action.
       *
       * Activation should effect the creation of a new simulation
       * that is ready to be started from the beginning.
       */
      QAction *stopAction;

      /**
       * The play action.
       *
       * The corresponding toolbarbutton respectively menuentry
       * should posses two states: playing and pause.
       * Activation should effect toggling between these states.
       */
      QAction *playAction;

      /**
       * The step action.
       *
       * Activation should effect a progress of the simulation
       * about one timesteps. How much time this is depends on
       * the simulation's stepsize.
       */
      QAction *stepAction;

      /**
       * The fast forward action.
       *
       * Activation should effect a progress of the simulation about 5 seconds,
       * independent from the actual stepsize.
       */
      QAction *fForwardAction;

      /**
       * The recording action.
       *
       * This action indicated whether the simualtion shall be recorded or not.
       */

      QAction *recordAction;

      /**
       * The alter movie settings action.
       */
      QAction *alterMovieSettingsAction;

      QAction *quitAction;

      /**
       * The iconset for the stop action.
       */
      QIconSet stopIcons;

      /**
       * The iconset for the play action.
       */
      QIconSet playIcons;

      /**
       * The iconset for the step action.
       */
      QIconSet stepIcons;

      /**
       * The iconset for the pause action.
       */
      QIconSet pauseIcons;

      /**
       * The iconset for the fast forward action.
       */
      QIconSet fForwardIcons;

      /**
       * The iconset for the record button.
       */
      QIconSet recordIcons;
      
      /**
       * The icon for the action with which the movie settings can be alter.
       */
      QIconSet recordingAllowedIcons;

      QIconSet recordingDisallowedIcons;

      QIconSet quitIcons;

      bool simulationRunning;

    public slots:

      void slotPlayPressed();

      void slotStopPressed();

      void slotRecordingAllowed( bool isAllowed );
  };

}

#endif // SIGEL_SLAVEGUI_SIG_SIMULATIONCONTROLS_H
