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
#ifndef SIGEL_MASTERGUI_SIG_EXPERIMENTVIEW_H
#define SIGEL_MASTERGUI_SIG_EXPERIMENTVIEW_H

#include "SIGEL_MasterGUI/SIG_ExperimentViewBase.h"

#include "SIGEL_GP/SIG_GPExperiment.h"


namespace SIGEL_MasterGUI
{
/**
 * The widget belonging to an experiment. Experiments can be started, paused or stopped here.
 *
 * In this widget some general information about the experiment can be viewed. Also a comment
 * for the experiment can be given here.
 */
class SIG_ExperimentView : public SIG_ExperimentViewBase
{ 
    Q_OBJECT

public:

    /**
     * The constructor of SIG_ExperimentView.
     *
     * Receives the SIG_GPExperiment it belongs to.
     * @param parent The parent widget of SIG_AllIndividualsView
     * @param name Internal name for Qt.
     * @param theExperiment A reference to the experiment this view belongs to.
     */
    SIG_ExperimentView( QWidget* parent, const char* name, Qt::WindowFlags fl, SIGEL_GP::SIG_GPExperiment &theExperiment );

    /**
     * The destructor.
     *
     * Will be erased if not needed.
     */
    ~SIG_ExperimentView();

    void putIntoExperiment();

    void getOutOfExperiment();

  public slots:

      void slotShowFitnesscurve();

      void slotExportPostScript();

      /**
      	*	This slot will be envoked when the user clicks on the history checkbox.
      	*/
      void slotHistory(bool selected);

      /**
      	*	This slot will be envoked when the user moves the autosave slider.
      	*/
      void slotIntervallChanged(int value);

 private:
    
    /**
     * The SIG_GPExperiment belonging to this experiment.
     */
    SIGEL_GP::SIG_GPExperiment &theExperiment;

    void streamToGnuPlot( QTextStream &stream );

};

}
#endif // SIGEL_MASTERGUI_SIG_EXPERIMENTVIEW_H
