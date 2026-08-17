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
#ifndef SIGEL_MASTERGUI_SIG_MAINWINDOW_H
#define SIGEL_MASTERGUI_SIG_MAINWINDOW_H

#include <qmainwindow.h>
#include <qsplitter.h>
#include <qpushbutton.h>
#include <qwidgetstack.h>
#include <qmenubar.h>
#include <qpopupmenu.h>
#include <qtoolbar.h>
#include <qaction.h>
#include <qlabel.h> // only for baseWidget which will be nicer someday...

#include "SIGEL_MasterGUI/SIG_ExperimentListView.h"

/**
 * This is the namespace in which all GUI-elements live.
 */

namespace SIGEL_MasterGUI
{
  /**
   * The class of the main window of the application.
   *
   * This window contains the menubar, the toolbar and the statusbar.
   * Furthermore it contains the main widget, which is the splitter.
   */
  class SIG_MainWindow : public QMainWindow
    {
      Q_OBJECT

    public:
      
      /**
       * The constructor of the main window.
       *
       * Here all menus are set up, as well as the splitter with the
       * SIG_ExperimentListView on one side and the QWidgetStack on the
       * other side.
       * @param parent The parent widget of SIG_MainWindow. If 0 (the normal case)
       * the widget will get its own window.
       * @param name Internal widget name for Qt.
       */
      SIG_MainWindow( QWidget * parent = 0, const char * name = 0, WFlags f = WType_TopLevel );

      /**
       * The destructor of SIG_MainWindow.
       */
      ~SIG_MainWindow();
      
   public slots:
	/**
	 * This slot is called when the about box has to be shown.
	 */
      void slotAbout();

   /**
    * This slot is used by the main window to indicate whether there
    * are open experiments or not.
    *
    * If isNotEmpty is false some menu items and toolbar items will be
    * disabled. This is done via a QActionGroup.
    */
      void slotShowEmpty( bool isNotEmpty );

      /**
       * This slot is called whenever the user is about to quit the application.
       */
      void slotAboutToQuit();

      void slotUseBigPixmaps();

      void slotUseTextLabels();

      void slotChangeFont();

   protected:
   /**
    * The file menu.
    *
    * Experiment can be created, deleted, loaded and saved here
    * for example.
    */
      QPopupMenu *fileMenu;

      /**
       * The help menu.
       */ 
      QPopupMenu *helpMenu;

      /**
       * The view menu.
       */
      QPopupMenu *viewMenu;

      /**
       * The optionsMenu;
       */
      QPopupMenu *optionsMenu;
      /**
       * The actions menu
       */
      QPopupMenu *individualsMenu;

      /**
       * The toolbar which contains the file actions.
       */
      QToolBar *fileToolBar;

      /**
       * The action group that is disabled when there is no
       * experiment present.
       */
      QActionGroup *noExperimentActionGroup;

      /**
       * The action group that is disabled when there is no
       * experiment present.
       */
      QActionGroup *evolutionRunningActionGroup;

      /**
       * The splitter which contains the two main widgets.
       *
       * The two main widgets are the SIG_ExperimentListView object
       * on the left side and the QWidgetStack on the right side
       * of the screen.
       */
      QSplitter *splitter;

      /**
       * The widget stack onto which the widgets are packed.
       *
       * When a new experiment is created, all widgets belonging to
       * this experiment are packed onto this widget stack. Whenever
       * a widget is needed to be shown, it is first looked up in the
       * widget dictionary, and the displayed on the widget stack.
       */
      QWidgetStack *widgetStack;

      /**
       * The widget that is displayed if there are no other widgets on
       * the widget stack.
       */
      QLabel *widgetBase;

      /**
       * Saves the menu item item for the menu item with which one can switch to
       * using big pixmaps or not.
       */
      int bigPixmapID;

      int textLabelsID;
      
      /**
       * The experiment list view. All experiments are shown in this widget.
       */
      SIGEL_MasterGUI::SIG_ExperimentListView *experimentListView;

	  /**
	   * The Meta-GP definitions
	   */
	  private:
		  QPopupMenu *mtMenu;
		  QToolBar	 *mtToolBar;
		  QAction	 *mtUseAction;
		  QAction	 *mtConfigureAction;
		  QAction	 *mtChoiceEvaluatorAction;
		  QAction	 *mtChoiceClassifierAction;
		  QActionGroup *mtChoiceTypeActionGroup;
	  public slots:
		  void slotMTUseMT(bool state);
		  void slotMTConfigureSystem();
		  void slotMTSwitchSystem(QAction *selSystem);
		  void slotActExpChanged();

    };
  
}

#endif // SIGEL_MASTERGUI_SIG_MAINWINDOW_H



