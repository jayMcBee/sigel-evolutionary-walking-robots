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
#ifndef SIGEL_MASTERGUI_SIG_GPPARAMETER_H
#define SIGEL_MASTERGUI_SIG_GPPARAMETER_H

#include "SIGEL_MasterGUI/SIG_GPParameterBase.h"

#include "SIGEL_GP/SIG_GPExperiment.h"

namespace SIGEL_MasterGUI
{

  /**
   * The class of the window, in which the genetic programming parameters can be altered.
   */
class SIG_GPParameter : public SIG_GPParameterBase
{ 
    Q_OBJECT

public:

    /**
     * The constructor of SIG_GPParameter.
     *
     * Receives the SIG_GPExperiment it belongs to.
     * @param parent The parent widget of SIG_AllIndividualsView
     * @param name Internal name for Qt.
     * @param theExperiment A reference to the experiment this view belongs to.
     */
    SIG_GPParameter( QWidget* parent, const char* name, Qt::WindowFlags fl, SIGEL_GP::SIG_GPExperiment &theExperiment );

    /**
     * The destructor.
     *
     * Will be erased if not needed.
     */
    ~SIG_GPParameter();
    
    /**
     * This slot is called whenever the values of the widgets shall be
     * put into the genetic programming parameters of the experiment.
     */
    void putIntoExperiment();

    /**
     * This slot is called whenever the values of the genetic programming
     * parameters shall be put into the widgets.
     */
    void getOutOfExperiment();

public slots:

      /**
       * This slot is called when the user clicks on the tool button to
       * select a graveyard dir.
       *
       * A file dialog pops up in which the user can select a directory.
       */
     void slotChangeGraveyardDir();

/**
 * This slot is called when the user clicks on the tool button to
 * select a graveyard dir.
 *
 * A file dialog pops up in which the user can select a directory.
 */
 void slotChangePoolImageDir();

 void slotAddHost();

 void slotEditHost();

 void slotDeleteHost();

 void slotEnableAllHosts();

 void slotDisableAllHosts();

 void slotItemDoubleClicked( QTreeWidgetItem * );

 void slotMutationChanged( int );

 void slotCrossoverChanged( int );

 void slotTourPerGenChanged( int );


 private:

 /**
  * The SIG_GPExperiment belonging to this experiment.
  */
 SIGEL_GP::SIG_GPExperiment &theExperiment;

 /**
  * This string saves the SIGEL_ROOT environment variable which will be needed several
  * times for pixmap-operations.
  */
 QString sigelRoot;
};

}

#endif // SIGEL_MASTERGUI_SIG_GPPARAMETER_H
