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
#ifndef SIGEL_MASTERGUI_SIG_LANGUAGEPARAMETERS_H
#define SIGEL_MASTERGUI_SIG_LANGUAGEPARAMETERS_H

#include <qpixmap.h>

#include "SIGEL_MasterGUI/SIG_LanguageParametersBase.h"

#include "SIGEL_GP/SIG_GPExperiment.h"

namespace SIGEL_MasterGUI
{

  /**
   * The class of the window, in which the language parameters can be altered.
   */
class SIG_LanguageParameters : public SIG_LanguageParametersBase
{ 
    Q_OBJECT

public:
    /**
     * The constructor of SIG_LanguageParameters.
     *
     * Receives the SIG_GPExperiment it belongs to.
     * @param parent The parent widget of SIG_AllIndividualsView
     * @param name Internal name for Qt.
     * @param theExperiment A reference to the experiment this view belongs to.
     */
    SIG_LanguageParameters( QWidget* parent, const char* name, Qt::WindowFlags fl, SIGEL_GP::SIG_GPExperiment &theExperiment );

    /**
     * The destructor.
     *
     * Will be erased if not needed.
     */
    ~SIG_LanguageParameters();
    
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

    void slotPushButtonEditClicked();

    void slotPushButtonAllowAllClicked();

    void slotPushButtonDisallowAllClicked();

    void slotCommandDoubleClicked( QTreeWidgetItem *theItem );

 private:

    /**
     * The SIG_GPExperiment belonging to this experiment.
     */
    SIGEL_GP::SIG_GPExperiment &theExperiment;

    QPixmap allow;
    QPixmap disallow;
    QString sigelRoot;
};

}

#endif // SIGEL_MASTERGUI_SIG_LANGUAGEPARAMETERS_H
