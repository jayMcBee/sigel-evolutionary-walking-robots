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
#ifndef SIGEL_MASTERGUI_SIG_ROBOTVIEW_H
#define SIGEL_MASTERGUI_SIG_ROBOTVIEW_H

#include "SIGEL_MasterGUI/SIG_RobotBase.h"

#include "SIGEL_GP/SIG_GPExperiment.h"

namespace SIGEL_MasterGUI
{

  /**
   * The class of the window, in which the robot can be viewed, loaded, etc...
   *
   * This class inherits from SIG_RobotBase which was designed with the Qt
   * designer.
   */
class SIG_RobotView : public SIG_RobotBase
{ 
    Q_OBJECT

public:
    /**
     * The constructor.
     */
    SIG_RobotView( QWidget* paren, const char* name, WFlags fl, SIGEL_GP::SIG_GPExperiment &theExperiment );

    /**
     * The destructor.
     *
     * Will be deleted if not used.
     */
    ~SIG_RobotView();

    void putIntoExperiment();
    
    void getOutOfExperiment();

 private:
    SIGEL_GP::SIG_GPExperiment &theExperiment;
};

}

#endif // SIGEL_MASTERGUI_SIG_ROBOTVIEW_H
