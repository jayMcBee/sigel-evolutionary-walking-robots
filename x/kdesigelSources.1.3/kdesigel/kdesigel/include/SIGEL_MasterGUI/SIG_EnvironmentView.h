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
#ifndef SIGEL_MASTERGUI_SIG_ENVIRONMENTVIEW_H
#define SIGEL_MASTERGUI_SIG_ENVIRONMENTVIEW_H

#include "SIGEL_MasterGUI/SIG_EnvironmentBase.h"

#include "SIGEL_GP/SIG_GPExperiment.h"

namespace SIGEL_MasterGUI
{

  /**
   * The class of the window, in which an environment can be viewed.
   */
class SIG_EnvironmentView : public SIG_EnvironmentBase
{ 
    Q_OBJECT

public:
    /**
     * The constructor of SIG_EnvironmentView.
     *
     * Receives the SIG_GPExperiment it belongs to.
     * @param parent The parent widget of SIG_AllIndividualsView
     * @param name Internal name for Qt.
     * @param theExperiment A reference to the experiment this view belongs to.
     */
    SIG_EnvironmentView( QWidget* parent, const char* name, WFlags fl, SIGEL_GP::SIG_GPExperiment &theExperiment );

    /**
     * The destructor.
     */
    ~SIG_EnvironmentView();
    
    /**
     * This slot is called whenever the values of the widgets shall be
     * put into the environment parameters of the experiment.
     */
    void putIntoExperiment();

    /**
     * This slot is called whenever the values of the environment parameters
     * shall be put into the widgets.
     */
    void getOutOfExperiment();

 public slots:

		/** this slot is called whenever a radiobutton in the general tab has been changed
			*/
		virtual void slotFloorSelectionChanged();
		
		/** this slot is called when the user wants to select a picture-file for the
			* dynamechs floor
			*/
		virtual void slotSelectFile();
		
    /**
  		*	This slot will be envoked when the user wants to select texture file
  		*/
  	virtual void slotSelectTextureFile();
  	
  	/**
  		*	This slot will be envoked when the user moves the alpha slider
  		*/
  	virtual void slotAlpha();
  	
  	/**
  		*	This slot will be envoked when the user check or uncheck the Texture checkbox
  		*/
		virtual	void slotTextureSelect();
			
 private:

    /**
     * A reference to the gp experiment.
     */
    SIGEL_GP::SIG_GPExperiment &theExperiment;
};

}
#endif // SIGEL_MASTERGUI_SIG_ENVIRONMENTVIEW_H
