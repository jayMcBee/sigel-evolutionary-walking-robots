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
#ifndef SIGEL_MASTERGUI_SIG_ALLINDIVIDUALSVIEW_H
#define SIGEL_MASTERGUI_SIG_ALLINDIVIDUALSVIEW_H

#include <qsplitter.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qlayout.h>
#include <qtimer.h>

#include "SIGEL_MasterGUI/SIG_IndividualList.h"
#include "SIGEL_MasterGUI/SIG_IndividualView.h"
#include "SIGEL_MasterGUI/SIG_AddIndividualsDialog.h"

#include "SIGEL_GP/SIG_GPExperiment.h"

// only needed for the zwischenziel!!!
#include "SIGEL_Tools/SIG_Randomizer.h"
#include "SIGEL_GP/SIG_GPPopulation.h"

namespace SIGEL_MasterGUI
{
  /**
   * This is the class of the widget in which the pool and single programs
   * can be viewed.
   */
  class SIG_AllIndividualsView : public QSplitter
    {
      Q_OBJECT

    public:
      /**
       * The constructor of SIG_AllIndividualsView.
       *
       * Receives the SIG_GPExperiment it belongs to.
       * @param parent The parent widget of SIG_AllIndividualsView
       * @param name Internal name for Qt.
       * @param theExperiment A reference to the experiment this view belongs to.
       */
      SIG_AllIndividualsView( QWidget * parent, const char * name, SIGEL_GP::SIG_GPExperiment &theExperiment );

      /**
       * The destructor.
       *
       * Will be erased if not needed.
       */
      ~SIG_AllIndividualsView();

      /**
       * This function returns the number of selected items in the listview.
       */
      int numberOfSelectedItems();

      /**
       * The list in the upper part of the SIG_AllIndividualsView.
       *
       * This is the widget which contains the list and some buttons.
       */
      SIG_IndividualList *individualList;
      
      /**
       * The individual view in the lower part of the view.
       */
      SIG_IndividualView *individualView;

      /**
       * The individual menu.
       *
       * This menu is shown when a user clicks on an
       * individual.
       */
      QPopupMenu *individualMenu;

      /**
       * The menu that popups if one clicks in the listview and
       * not on an item.
       */
      QPopupMenu *listviewMenu;
      
    signals:
      void signalDataRefreshNeeded();

    public slots:

    /**
     * This slot is called for example when an experiment was loaded and
     * the list needs a complete refresh.
     */
      void slotCompleteRefreshList();

    /**
     * This slot is called whenever individuals should be added.
     */
      void slotAddIndividuals();

      /**
       * This slot is called whenever individuals should be deleted.
       */ 
      void slotDeleteIndividuals();

      /**
       * This slot is called to reset all individuals in the pool to -1.
       */
      void slotResetPool();
      
      /**
       * This slot is called whenever the pushbutton Stats is clicked.
       */
      void slotStatsClicked();

      /**
       * This slot is called whenever one clicks the right mouse button in
       * the individual list (the list displaying the pool).
       */
      void slotRightButtonClicked( QListViewItem *theItem, const QPoint &thePoint, int inside );
      
      /**
       * This slot is called whenever one doubleclicks on an item in the
       * individual list.
       */
      void slotDoubleClicked( QListViewItem * theItem );

      /**
       * This slot is called whenever individuals shall
       * be visualized.
       */
      void slotVisualize();

      /**
       * This slot is called when the selection in the individual list changes.
       */
      void slotSelectionChanged();

      void slotEvolutionNotRunning( bool isNotRunning );

      void slotImportProgram();

      void slotExportProgram();

      void slotImportIndividual();

      void slotExportIndividual();

    private:
      
      /**
       * A reference to the experiment object.
       */
      SIGEL_GP::SIG_GPExperiment &theExperiment;
    };
}

#endif // SIGEL_MASTERGUI_SIG_ALLINDIVIDUALSVIEW_H
