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
#ifndef SIGEL_MASTERGUI_SIG_EXPERIMENTLISTVIEW_H
#define SIGEL_MASTERGUI_SIG_EXPERIMENTLISTVIEW_H

#include <QTreeWidget>
#include <QStackedWidget>
#include <QHash>
#include <QMenu>

#include "SIGEL_MasterGUI/SIG_GUIGPExperiment.h"

namespace SIGEL_MasterGUI
{

  /**
   * The class that lets the user view the experiments.
   */

class SIG_ExperimentListView : public QTreeWidget
{
  Q_OBJECT
 public:

  /**
   * The constructor of the experiment list view.
   *
   * @param parent The parent widget. If 0 the widget gets
   * its own window.
   * @param name The internal name of the widget.
   * @param theWidgetStack The widget stack in the main window.
   */
  SIG_ExperimentListView( QWidget * parent, const char * name, QStackedWidget *theWidgetStack );

  /**
   * The destructor of the SIG_ExperimentListView.
   *
   * Will be erased if not needed.
   */
  ~SIG_ExperimentListView();

  /**
   * Returns the experiment object of the experiment called name.
   *
   * @param name The name of the experiment.
   * @return The experiment called name.
   */
  SIG_GUIGPExperiment* getByExperimentName( QString name );

  /**
   * Checks whether an experiment exists under name.
   *
   * If this is the case, something has to be done because no
   * two experiments shall have the same name.
   * @param name The experiment name that shall be checked.
   * @return A boolean indicating whether the experiment name exists.
   */
  bool experimentExists( QString name);

  /**
   * This function is called when there is already an experiment named existingName.
   *
   * The function returns an alternative name for the experiment.
   *
   * This function is called whenever an experiment already exists under
   * existingName.
   * @pre An experiment exists under existingName.
   * @post A new and unique name is returned.
   */
  QString getAlternativeName( QString existingName );

  /**
   * This method return a pointer to the currently seleted SIGEL_MasterGUI::SIG_GUIGPExperiment.
   *
   * If there is no experiment selected 0 is returned.
   */
  SIG_GUIGPExperiment* currentlySelectedExperiment();

  /**
   * Returns true while the evolution of any experiment in this list runs.
   *
   * The run checks ask this, not a single experiment.
   */
  bool isRunning();

  /**
   * This function returns the experiment name of the currently selected experiment.
   *
   * If there is no experiment selected a null-string ( QString()) is returned.
   * @return An alternative unique name for the experiment.
   */
  QString currentlySelectedExperimentName();

  /**
   * This function sets the selected item in the list view
   * to the item labelled label.
   *
   * @param label The label of the item that should be selected.
   */
  void selectItem( QString label );

  /**
   * The popup-menu that is shown whenever the user right clicks into the list view
   * where all experiments are shown.
   */
  QMenu *experimentListViewMenu;
  
 public slots:

    /**
     * Slot that is called when "New Experiment" was selected.
     */
  void slotNewExperiment();

 /**
  * Slot that is called whenever an experiment shall be renamed.
  */
 void slotRenameExperiment();

 /**
  * Slot that is called when an experiment should be deleted.
  */
  void slotDeleteExperiment();

  /**
   * Slot that is called when an experiment should be loaded.
   */
  void slotLoadExperiment();

  /**
   * Slot that is called when an experiment should be saved to its own file.
   * An experiment without a file asks for one, as slotSaveExperimentAs does.
   */
  void slotSaveExperiment();

  /**
   * Slot that is called when an experiment should be saved under a new file name.
   */
  void slotSaveExperimentAs();
  
  /**
   * This slot is needed so one can select the GP parameters via the main menu.
   */
  void slotShowGPParameters();
  
  /**
   * This slot is needed so one can select the simulation parameters via the main menu.
   */
  void slotShowSimulationParameters();

  /**
   * This slot is needed so one can select the simulation parameters via the main menu.
   */
  void slotShowLanguageParameters();

  /**
   * This slot is needed so one can select the robot view via the main menu.
   */
  void slotShowRobot();
  
  /**
   * This slot is needed so one can select the environment view via the main menu.
   */
  void slotShowEnvironment();
  
  /**
   * This slot is needed so one can select the individual view via the main menu.
   */
  void slotShowIndividuals();

  /**
   * The slot that is called whenever the selection changes in the list view.
   *
   * The method looks up the experiment name and searches for the experiment
   * in the list. Then it looks up the text at position 0 in the passed item
   * and calles slotSelectionChanged( QString ) in the SIG_GUIGPExperiment object.
   * @pre The item is not null.
   * @post The widget belonging to the list view item is shown in the
   * widget stack.
   * @param theItem The actual selected item.
   */
  void slotSelectionChanged( QTreeWidgetItem * theItem );

  void slotGPParametersImport();
  
  void slotSimulationParametersImport();

  void slotRobotImport();

  void slotLanguageParametersImport();

  void slotPopulationImport();

  void slotEnvironmentImport();

  void slotGPParametersExport();
  
  void slotSimulationParametersExport();

  void slotLanguageParametersExport();
  
  void slotPopulationExport();

  void slotEnvironmentExport();

  void slotGNUPlotExport();

  void slotAddIndividuals();

  void slotDeleteIndividuals();

  void slotResetAllFitnessValues();

  void slotVisualizeIndividuals();

  void slotProgramExport();

  void slotIndividualExport();

  void slotProgramImport();

  void slotIndividualImport();

 protected slots:

    /**
     * This slot is connected to the rightButtonClicked()-signal of the
     * experiment list view.
     *
     * First the method checks inside to see whether the click was onto a list view
     * item or outside. If it is outside the experimentListViewMenu is show. Else
     * the SIG_GUIGPExperiment object belonging to this item is searched. If it is found
     * the method slotRightClick() is called in the experiment object.
     */
  void slotRightButtonClicked( const QPoint & pos );

 signals:

 /**
  * This signal is emitted to signal whether the list view is empty or not.
  *
  * isNotEmpty is true if the list view is not empty and false if if is.
  */
  void isNotEmpty( bool isNotEmpty );

  /**
   * This signal is emitted to tell the main window whether the evolution is running.
   *
   * evolutionNotRunning is true if the evolution is not running.
   */
  void evolutionNotRunning( bool evolutionNotRunnning );

  /**
   * This signal is emitted if the selection changes.
   * It tells the main window wether the currently selected experiment
   * makes use of the meta gp-system.
   */
  void currentExperimentChanged();

 protected:

 /**
  * The pointer to the widget stack in the main window.
  *
  * This pointer is stored, so that it can be passed to a newly created
  * SIG_GUIGPExperiment.
  */
  QStackedWidget *widgetStack;

  /**
   * The dictionary in which all administered experiments are stored.
   *
   * Names have to be unique. Although it is technically possible to store
   * things under the same name in a QHash, for other reason the names are
   * needed to be unique.
   */
  QHash<QString, SIG_GUIGPExperiment *> experimentDict;

  /**
   * The number a new experiment gets.
   *
   * This number is incremented each time a new experiment is created.
   * If we create the n-th experiment it gets the name "Experiment-n".
   */
  int numberOfExperiments;
};

}

#endif SIGEL_MASTERGUI_SIG_EXPERIMENTLISTVIEW_H
