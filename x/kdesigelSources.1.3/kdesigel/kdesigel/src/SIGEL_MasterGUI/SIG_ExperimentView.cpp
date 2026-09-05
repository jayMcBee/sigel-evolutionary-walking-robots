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
#include "SIGEL_MasterGUI/SIG_Experiment.h"   // D29: anyEvolutionRunning()
#include <qpushbutton.h>
#include <qtextstream.h>
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qslider.h>
#include <qlcdnumber.h>
#include <qcheckbox.h>

#include "SIGEL_MasterGUI/SIG_ExperimentView.h"
#include "SIGEL_GP/SIG_GPExperimentHistoryEntry.h"
#include "SIGEL_Tools/SIG_IO.h"

#ifndef _WINDOWS
#include <unistd.h>
#endif

namespace SIGEL_MasterGUI
{

/* 
 *  Constructs a SIG_ExperimentView which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
SIG_ExperimentView::SIG_ExperimentView( QWidget* parent,  const char* name, Qt::WindowFlags fl, SIGEL_GP::SIG_GPExperiment &theExperiment )
  : SIG_ExperimentViewBase( parent, name, fl ), theExperiment( theExperiment )
{
}


/*  
 *  Destroys the object and frees any allocated resources
 */
SIG_ExperimentView::~SIG_ExperimentView() {
    // no need to delete child widgets, Qt does it all for us
}

void SIG_ExperimentView::putIntoExperiment() {
  // D29, and this is the path the first version of the guard MISSED.
  // slotStartEvolution disables gpParameter, simulationParameter, robotView,
  // languageParameters and environmentView -- but NOT experimentView, which is
  // the page the user is looking at when they press Start and which stays
  // fully live for the whole run. Its history checkbox and autosave slider are
  // wired straight to this function, not to putAllIntoExperiment, so guarding
  // only that one left the live path open.
  //
  // It is a live path in the strong sense: the running GP reads
  // getAutosave() every generation (SIG_GPManager.cpp:804-806) to decide
  // whether to save, and getHistory() decides what that save writes.
  //
  // The LCD read below is display-only and is deliberately AHEAD of the
  // guard. poolGeneration IS incremented per generation inside the loop
  // (SIG_GPManager.cpp:736), and 1.3 has no guard at all, so on 1.3 a page
  // switch during a run refreshes this display to the advanced value.
  // Returning before it would have made the port show a stale number where
  // 1.3 shows a fresh one -- a divergence the run lock would have
  // introduced, in a function whose point is to change nothing the user did
  // not ask for. Reading is not writing; the guard belongs below it.

  // update the generations display + progress-bar
  lcdnumberGenerations->display(theExperiment.population.getPoolGeneration());
  // generationProgBar

  if ( SIG_Experiment::anyEvolutionRunning() )
    return;

  // put comment into the box dedicated to the comment !
  theExperiment.comment = multilineeditComment->toPlainText();

  // autosave slider
  theExperiment.environment.setAutosave(lcdnumberAutosave->intValue());
  // history checkbox
  theExperiment.population.setHistory(checkboxHistory->isChecked());

};

void SIG_ExperimentView::getOutOfExperiment() {
  multilineeditComment->setText( theExperiment.comment );
  // autosave slider
  lcdnumberAutosave->display(theExperiment.environment.getAutosave());
  // history checkbox
  checkboxHistory->setChecked(theExperiment.population.getHistory());
};

void SIG_ExperimentView::streamToGnuPlot( QTextStream &stream ) {
  stream << "set data style lines\n"
	 << "set title \"Maximal, minimal and average fitnessvalues\"\n"
	 << "set xlabel 'Generation'\n"
	 << "set ylabel 'Fitnessvalue'\n";

  stream << "plot '-' title 'Maximal fitness', '-' title 'Minimal fitness', '-' title 'Average fitness'\n";

  for ( SIGEL_GP::SIG_GPExperimentHistoryEntry *actEntry : theExperiment.experimentHistory )
    {
      stream << actEntry->getGenerationNo()
		 << " "
		 << actEntry->getMaxFitness()
		 << "\n";
    }

  stream << "e\n";

  for ( SIGEL_GP::SIG_GPExperimentHistoryEntry *actEntry : theExperiment.experimentHistory )
    {
      stream << actEntry->getGenerationNo()
		 << " "
		 << actEntry->getMinFitness()
		 << "\n";
    }

  stream << "e\n";

  for ( SIGEL_GP::SIG_GPExperimentHistoryEntry *actEntry : theExperiment.experimentHistory )
    {
      stream << actEntry->getGenerationNo()
		 << " "
		 << actEntry->getAverageFitness()
		 << "\n";
    }

  stream << "e\n";
};

void SIG_ExperimentView::slotExportPostScript() {
  if (theExperiment.experimentHistory.isEmpty())
    {
      QMessageBox::information( this, "Info", "There is no evolution data to plot!", "Ok" );
      return;
    };

  QString fileName = QFileDialog::getSaveFileName( this, QString(), QString(), "Encapsulated postscript files (*.eps);;All files (*)" );

  if (fileName.isNull())
    return;

#ifdef _WINDOWS
/* The windows pipe support doesn't work as expected, so we have to use a
   a temporary file as input to gnuplot. */
   if(fileName.indexOf(".eps", -5, Qt::CaseInsensitive) == -1){
   	fileName += ".eps";
   }

   QString gnuCmdLine;
   gnuCmdLine  = ::getenv("SIGEL_ROOT");
   gnuCmdLine += "\\tmpFStat.plt";

   QFile pipeFile(gnuCmdLine);
   pipeFile.open(QIODevice::WriteOnly);

   QTextStream pipeStream( &pipeFile );

   pipeStream  << "set terminal postscript\n"
   				<< "set output \"" << fileName << "\"\n";
   streamToGnuPlot( pipeStream );
   pipeStream << "quit\n";

   pipeFile.close();

   gnuCmdLine += "\"";
   gnuCmdLine.prepend("gnuplot.exe \"");
   if(WinExec(gnuCmdLine.toLatin1().constData(), SW_SHOW) < 32){
   	// Qt 2 took the button LABEL here (button0Text); Qt 6 takes StandardButtons.
      QMessageBox::warning(this, "Error!", "Couldn't start gnuplot!", QMessageBox::Ok);
   	return;
   }
   pipeFile.remove();

#else

  FILE *gnuPlotStdInPipe = popen( "gnuplot -persist -", "w" );

  if (gnuPlotStdInPipe==NULL)
    {
      QMessageBox::warning( this, "Error!", "Couldn't start gnuplot!", "Ok" );
      return;
    };

  QFile pipeFile;
  pipeFile.open( gnuPlotStdInPipe, QIODevice::WriteOnly );

  QTextStream pipeStream( &pipeFile );

  pipeStream << "set terminal postscript\n"
	     << "set output '" << fileName << "'\n";

  streamToGnuPlot( pipeStream );

  pipeStream << "quit\n";

  pipeFile.close();

  pclose( gnuPlotStdInPipe );

#endif
};

void SIG_ExperimentView::slotShowFitnesscurve() {
  if (theExperiment.experimentHistory.isEmpty())
    {
      QMessageBox::information( this, "Info", "There is no evolution data to plot!", "Ok" );
      return;
    };

#ifdef _WINDOWS
/* The windows pipe support doesn't work as expected, so we have to use a
   a temporary file as input to gnuplot.
*/
   QString gnuCmdLine;
   gnuCmdLine  = ::getenv("SIGEL_ROOT");
   gnuCmdLine += "\\tmpFStat.plt";

   QFile pipeFile(gnuCmdLine);
   pipeFile.open(QIODevice::WriteOnly);

   QTextStream pipeStream( &pipeFile );

   streamToGnuPlot( pipeStream );
   pipeStream << "quit\n";
   pipeFile.close();

   gnuCmdLine += "\" - ";
   gnuCmdLine.prepend("gnuplot.exe \"");
   if(WinExec(gnuCmdLine.toLatin1().constData(), SW_SHOW) < 32){
   	// Qt 2 took the button LABEL here (button0Text); Qt 6 takes StandardButtons.
      QMessageBox::warning(this, "Error!", "Couldn't start gnuplot!", QMessageBox::Ok);
   	return;
   }
   pipeFile.remove();

#else
  //errno = 0;
  //signal(SIGPIPE,sigelSignalStandardHandler);
  FILE *gnuPlotStdInPipe = popen( "gnuplot -persist -", "w" );

  if (gnuPlotStdInPipe==NULL ) {
    QMessageBox::warning( this, "Error!", "Couldn't start gnuplot!", "Ok" );
    return;
  };

  QFile pipeFile;
 pipeFile.open( gnuPlotStdInPipe, QIODevice::WriteOnly );

  QTextStream pipeStream( &pipeFile );

  streamToGnuPlot( pipeStream );

  pipeStream << "quit\n";

  pipeFile.close();

  pclose( gnuPlotStdInPipe );
#endif
};

void SIG_ExperimentView::slotHistory(bool selected) {
  putIntoExperiment();
};

void SIG_ExperimentView::slotIntervallChanged(int value) {
  if (theExperiment.getPath()!="new") {
    lcdnumberAutosave->display(value);
    putIntoExperiment();
  }
  else {
    // this is because a new experiment has no path where it is saved
    // so no autosaving can be done
    sliderIntervall->setValue(0);
    QMessageBox::warning( this, "Error!", "You have to save the experiment first!", "Ok" );
  }
};

}
