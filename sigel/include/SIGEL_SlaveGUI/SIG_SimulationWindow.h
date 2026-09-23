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
#ifndef SIGEL_SLAVEGUI_SIG_SIMULATIONWINDOW_H
#define SIGEL_SLAVEGUI_SIG_SIMULATIONWINDOW_H

#include <QMainWindow>
#include <QToolBar>

#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"
#include "SIGEL_Program/SIG_Program.h"
#include "SIGEL_SlaveGUI/SIG_SimulationWidget.h"
#include "SIGEL_SlaveGUI/SIG_SimulationControls.h"

/**
 * This namespace (package) contains all widget classes of the
 * sigel_slave program.
 *
 * In fact all widgets are part of the window which visualizes
 * a simulation.
 */
namespace SIGEL_SlaveGUI
{

  /**
   * This is the mainwindow of the simulation visualisation.
   *
   * It offers an interface to feed the internal simulation visualisation
   * with the used simulatiom data.
   * It contains a toolbar to easily control the encapsulated simulation.
   * The main components of this window can be found in the class
   * SIG_SimulationWidget, which is the central widget of this QMainWindow.
   */
  class SIG_SimulationWindow : public QMainWindow
  {
    Q_OBJECT

  public:

    /**
     * The constructor of the SIG_SimulationWindow.
     *
     * Creates this classes components (for exmaple SIG_SimulationWidget and toolbar).
     *
     * @param parent Pointer to this widget's parent.
     * @param name This widget's name.
     * @param f The widget flag of this widget.
     */
    SIG_SimulationWindow( QWidget *parent,
			  char const *name = 0,
			  Qt::WindowFlags f = Qt::Window );

    /**
     * The destructor of the SIG_SimulationWindow.
     */
    ~SIG_SimulationWindow();

    /**
     * Calls the method visualizeThis of the enclosed SIG_SimulationWidget
     * (which is this QMainWindow's central widget) with the same
     * parameters.
     *
     * @param robot The robot that should take part in the simulation.
     * @param environment The environment that should take part in the simulation.
     * @param simulationParameters The parameters of the simulation.
     * @param program The program that should control the robot in the simulation.
     */
    void visualizeThis(SIGEL_Robot::SIG_Robot const &robot,
		       SIGEL_Environment::SIG_Environment const &environment,
		       SIGEL_Simulation::SIG_SimulationParameters const &simulationParameters,
		       SIGEL_Program::SIG_Program const &program);

    public slots:
      void slotStopPressed();

  protected:

    /**
     * Pauses the simulation while a modal dialog blocks this window.
     * Play keeps the event loop full, and the dialog would not be drawn.
     */
    bool event( QEvent *e ) override;

  private:

    /**
     * This SIG_SimulationWidget becomes the central widget of this
     * QMainWindow.
     */
    SIG_SimulationWidget *simulationWidget;

    /**
     * The toolbar that contains the simulation controls.
     */
    QToolBar *simulationControlBar;

    /**
     * The simulation controls that are attached in the toolbar and the menubar.
     */
    SIG_SimulationControls *simulationControls;

  };

}

#endif // SIGEL_SLAVEGUI_SIG_SIMULATIONWINDOW_H
