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
#ifndef SIGEL_MASTERGUI_SIG_EXPERIMENT_H
#define SIGEL_MASTERGUI_SIG_EXPERIMENT_H

#include <qwidgetstack.h>
#include <qstring.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qdict.h>
#include <qmultilineedit.h>

#include "SIGEL_MasterGUI/SIG_GPParameter.h"
#include "SIGEL_MasterGUI/SIG_SimulationParameter.h"
#include "SIGEL_MasterGUI/SIG_EnvironmentView.h"
#include "SIGEL_MasterGUI/SIG_RobotView.h"
#include "SIGEL_MasterGUI/SIG_ExperimentView.h"
#include "SIGEL_MasterGUI/SIG_ExperimentItem.h"
#include "SIGEL_MasterGUI/SIG_AllIndividualsView.h"
#include "SIGEL_MasterGUI/SIG_LanguageParameters.h"

#include "SIGEL_GP/SIG_GPExperiment.h"

namespace SIGEL_GP
{ class SIG_GUIGPManager; };

namespace SIGEL_MasterGUI
{
  /**
   * This class encapsules an experiment on GUI-side.
   *
   * It holds one SIG_GPExperiment as well as one SIG_GPManager.
   */
  class SIG_Experiment : public QWidget
    {
      Q_OBJECT
	
	public:

      /**
       * The constructor of SIG_Experiment.
       *
       * Receives the SIG_GPExperiment it belongs to.
       * @param parent The parent widget of SIG_AllIndividualsView
       * @param name Internal name for Qt.
       * @param theExperiment A reference to the experiment this view belongs to.
       */
      SIG_Experiment( QString name, QWidgetStack *theWidgetStack, SIG_ExperimentItem *theExperimentItem );
      
      /**
       * The destructor.
       *
       * Will be erased if not needed.
       */
      ~SIG_Experiment();
  
      /**
       * This function return the name of the experiment.
       * @return The name of the experiment.
       */
      QString getName() const;

      /**
       * This function set the name of the experiment to a new name.
       *
       * Position 0 of the experimentItem is set to new name as well as the variable.
       * Also the experimentView in the widget stack will be reinserted under newName.
       * @param newName The new name of the experiment.
       */
      void setName( QString newName );

      /**
       * The SIG_GPExperiment belonging to this experiment.
       */
      SIGEL_GP::SIG_GPExperiment gpExperiment;

      /**
       * The SIG_GPManager belonging to this experiment.
       */
      SIGEL_GP::SIG_GUIGPManager *gpManager;
      
      public slots:

	/**
	 * This function is called to put all the data out of the widget into
	 * the experiment.
	 *
	 * The function calles the function putIntoExperiment() for each widget in this class.
	 */
	void putAllIntoExperiment();
      
      /**
       * This function is called to read all data out of the experiment to
       * put it into the corresponding widgets.
       *
       * The function calles the function getOutOfExperiment() for each widget in this class.
       */
      void getAllOutOfExperiment();
      
      /**
       * This slot is called by the experiment list view, whenever the user
       * performs a right click onto an item.
       *
       * option is the text of the item that was clicked and thePoint is the
       * point where the right click was performed. it is also the point
       * where the menu is shown of course.
       * @param option The text of the clicked item.
       * @param thePoint The point where the click was performed.
       */
      void slotRightClick( QString option, const QPoint & thePoint );
      
      /**
       * This slot is called by the experiment list view, whenever the
       * selection changes.
       *
       * the widget in the widget-dictionary saved under option is shown.
       */
      void slotSelectionChanged( QString option );
      
      /**
       * This slot is called whenever the evolution shall be started.
       */
      void slotStartEvolution();
      
      /**
       * This slot is called to stop the evolution.
       */
      void slotStopEvolution();

      /**
       * This slot is used to import simulation parameter.
       */
      void slotSimulationParameterImport();
      
      /**
       * This slot is used to export simulation parameter.
       */
      void slotSimulationParameterExport();

      /**
       * This slot is used to import an environment.
       */
      void slotEnvironmentImport();
      
      /**
       * This slot is used to export an environment.
       */
      void slotEnvironmentExport();
      
      /**
       * This slot is used to import gp parameters.
       */
      void slotGPParameterImport();
      
      /**
       * This slot is used to export gp parameters.
       */
      void slotGPParameterExport();

      /**
       * This slot is used to import language parameters.
       */
      void slotLanguageParameterImport();
      
      /**
       * This slot is used to export language parameters.
       */
      void slotLanguageParameterExport();

      /**
       * This slot is used to import a population.
       */
      void slotPopulationImport();
      
      /**
       * This slot is used to export a population.
       */
      void slotPopulationExport();
      
      /**
       * This slot is called to import a robot.
       *
       * Importing a robot means to load a robot that is completely unprocessed. For examples the tensors of the links are
       * not yet calculated.
       */
      void slotRobotImport();

      void slotGNUPlotExport();


      /**
       * This slot gives information about the robot.
       *
       */
      void slotRobotInfo();

      /**
       * This slot load an already processed robot.
       *
       * Only already processed robots can be loaded.
       */
      void slotRobotLoad();

      /**
       * This slot saves a robot.
       *
       * To save a robot it must either first be imported or loaded. So in every case is was already processed.
       */
      void slotRobotSave();

      /**
       * This slot is called whenever the evolution stops.
       */
      void slotEvolutionStopped();

    signals:
      void signalEvolutionNotRunning( bool );

    protected:

      /**
       * This function checks if a filename has a given ending.
       * @param fileName The file name to check.
       * @param ending The ending to be checked for.
       */
      QString checkEnding( QString fileName, QString ending );

      /**
       * All widgets belonging to an experiment are stored in this dictionary.
       */
      QDict<QWidget> widgetDict;

      /**
       * All menus belonging to an experiment are stored in this dictionary.
       */
      QDict<QPopupMenu> menuDict;

      /**
       * The experiment name.
       */
      QString experimentName;

      /**
       * A pointer to the widget stack in the main window.
       */
      QWidgetStack *widgetStack;

    public:
      
      /**
       * The popup menu belonging to the gp parameters.
       *
       * This menu is shown whenever the user rightclicks onto the GP-Parameter
       * item in the experiment list view.
       */
      QPopupMenu *menuGPParameter;

      /**
       * The popup menu belonging to the simulation parameters.
       *
       * This menu is shown whenever the user rightclicks onto the Simulation-Parameter 
       * item in the experiment list view.
       */
      QPopupMenu *menuSimulationParameter;

      /**
       * The popup menu belonging to the robot view.
       *
       * This menu is shown whenever the user rightclicks onto the robot view
       * item in the experiment list view.
       */
      QPopupMenu *menuRobotView;

      /**
       * The popup menu belonging to the environment view.
       *
       * This menu is shown whenever the user rightclicks onto the environment view
       * item in the experiment list view.
       */
      QPopupMenu *menuEnvironmentView;

      /**
       * The popup menu belonging to the experiment view.
       *
       * This menu is shown whenever the user rightclicks onto the experiment view
       * item in the experiment list view.
       */
      QPopupMenu *menuExperimentView;
      
      /**
       * The SIG_ExperimentItem belonging to the experiment.
       */
      SIG_ExperimentItem *experimentItem;

      /**
       * The SIG_GPParameter belonging to the experiment.
       */
      SIG_GPParameter *gpParameter;

      /**
       * The SIG_SimulationParameter belonging to the experiment.
       */
      SIG_SimulationParameter *simulationParameter;

      /**
       * The SIG_EnvironmentView belonging to the experiment.
       */
      SIG_EnvironmentView *environmentView;

      /**
       * The SIG_RobotView belonging to the experiment.
       */
      SIG_RobotView *robotView;

      /**
       * The SIG_ExperimentView belonging to the experiment.
       */
      SIG_ExperimentView *experimentView;

      /**
       * The widget in which all the individuals can be viewed.
       */
      SIG_AllIndividualsView *allIndividualsView;

      /**
       * The widget in which the language parameters can be set.
       */
      SIG_LanguageParameters *languageParameters;
    };
  
}
#endif // SIGEL_MASTERGUI_SIG_EXPERIMENT_H

