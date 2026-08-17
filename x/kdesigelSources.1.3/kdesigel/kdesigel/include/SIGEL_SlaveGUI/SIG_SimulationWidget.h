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
#ifndef SIGEL_SLAVEGUI_SIG_SIMULATIONWIDGET_H
#define SIGEL_SLAVEGUI_SIG_SIMULATIONWIDGET_H

#include "SIGEL_SlaveGUI/SIG_SimulationWidgetBase.h"

#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Environment/SIG_Environment.h"
#include "SIGEL_Simulation/SIG_SimulationParameters.h"
#include "SIGEL_Program/SIG_Program.h"

namespace SIGEL_SlaveGUI
{

  /**
   * This is the central widget of the class SIG_SimulationWindow.
   *
   * It inherits from the class SIG_SimulationWidgetBase which
   * was created with the QT Designer. Due to this fact most
   * GUI features can be found and edited in the file
   * SIG_SimulationWidgetBase.ui.
   */
  class SIG_SimulationWidget : public SIG_SimulationWidgetBase
  {
    Q_OBJECT

  public:

    /**
     * The constructor of the SIG_SimulationWidget.
     *
     * @param parent Pointer to this widget's parent.
     * @param name This widget's name.
     * @param f The widget flag of this widget.
     */
    SIG_SimulationWidget( QWidget *parent=0,
			  char const *name=0,
			  WFlags f=0 );

    /**
     * The destructor of the SIG_SimulationWidget.
     */
    ~SIG_SimulationWidget();

  public slots:

    /**
     * Sets the yaw value of the contained SIG_SimulationVisualisationWidget
     * visualisationWidget by calling its method setYaw.
     */
    void slotSetYaw( int yaw );

    /**
     * Sets the pitch value of the contained SIG_SimulationVisualisationWidget
     * visualisationWidget by calling its method setPitch.
     */
    void slotSetPitch( int pitch );

    /**
     * Sets the distance value of the contained SIG_SimulationVisualisationWidget
     * visualisationWidget by calling its method setDistance.
     */
    void slotSetDistance( int distance );

    /**
     * Changes the values of the enclosed yaw- and pitch-sliders.
     *
     * This indirectly effects the calling of the slots slotSetYaw
     * and slotSetPitch.
     *
     * @param deltaX The amount to change the yaw value.
     * @param deltaY The amount to change the pitch value.
     */
    void slotMouseRotation( int deltaX, int deltaY );

    /**
     * Changes the value of the enclosed distance-slider.
     *
     * This indirectly effects the calling of the slot slotSetDistance.
     *
     * @param deltaY The amount to change the distance value.
     */
    void slotMouseZoom( int deltaY );

    void slotSetSimulationTime( QTime simulationTime );

    void slotSetTraceRobot( bool newValue );

    void slotSetPosition( DL_vector position );

    void slotStopSimulation();

    /**
     * Calls the method visualizeThis of the enclosed
     * SIG_SimulationVisualisationWidget visualisationWidget
     * with the same parameters.
     */
    void visualizeThis(SIGEL_Robot::SIG_Robot const &robot,
		       SIGEL_Environment::SIG_Environment const &environment,
		       SIGEL_Simulation::SIG_SimulationParameters const &simulationParameters,
		       SIGEL_Program::SIG_Program const &program);

  };

}

#endif // SIGEL_SLAVEGUI_SIG_SIMULATIONWIDGET_H
