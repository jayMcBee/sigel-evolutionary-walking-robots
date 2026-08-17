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
#ifndef SIGEL_MASTERGUI_SIG_INDIVIDUALVIEW_H
#define SIGEL_MASTERGUI_SIG_INDIVIDUALVIEW_H

#include "SIGEL_MasterGUI/SIG_IndividualViewBase.h"

#include "SIGEL_GP/SIG_GPIndividual.h"

namespace SIGEL_MasterGUI
{

  /**
   * The class of the window, in which a single individual (program) can be viewed.
   *
   * This class is used in two ways. In one case in the SIG_AllIndividualsView to
   * display information about the currently selected individual. In the other case
   * it is used after a double click to open a new window displaying the information
   * about the individual.
   */
class SIG_IndividualView : public SIG_IndividualViewBase
{ 
    Q_OBJECT

public:

    /**
     * The constructor for creating an empty individual view.
     *
     * @param parent The parent widget of this individual view. If 0 the individual
     * view will get its own window.
     * @param name The internal name used by Qt.
     */
    SIG_IndividualView( QWidget* parent = 0, const char* name = 0, WFlags fl = 0 );

    /**
     * The constructor with a supplied individual.
     *
     * This constructor is used if one wants to show a certain
     * individual (for example after a double click in the
     * individual list).
     * @param parent The parent widget of this individual view. If 0 the individual
     * view will get its own window.
     * @param name The internal name used by Qt.
     * @param theGPIndividual The individual that shall be displayed.
     */
    SIG_IndividualView( QWidget* parent, const char* name, WFlags fl, SIGEL_GP::SIG_GPIndividual *theGPIndividual );

    /**
     * The destructor.
     *
     * Will be erased if not needed.
     */
    ~SIG_IndividualView();

    /**
     * This method clear the individual view.
     */
    void clear();

 protected:
    
    /**
     * This event needed to be reimplemented.
     *
     * As individual list windows can be created dynamically
     * by double clicking on an individual in the individual
     * list they have to take care for themselves, so that
     * they are destroyed when no longer needed. This
     * reimplementation takes care of this.
     */
    // void closeEvent( QCloseEvent *e);
};

}

#endif // SIGEL_MASTERGUI_SIG_INDIVIDUALVIEW_H
