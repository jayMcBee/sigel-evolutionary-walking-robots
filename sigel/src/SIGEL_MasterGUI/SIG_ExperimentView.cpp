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
#include "SIGEL_MasterGUI/SIG_ExperimentListView.h"
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

#include <unistd.h>

namespace SIGEL_MasterGUI
{

/* 
 *  Constructs a SIG_ExperimentView which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
SIG_ExperimentView::SIG_ExperimentView( QWidget* parent,  const char* name, Qt::WindowFlags fl, SIGEL_GP::SIG_GPExperiment &theExperiment, SIG_GUIGPExperiment &guiExperiment )
  : SIG_ExperimentViewBase( parent, name, fl ), theExperiment( theExperiment ), guiExperiment( guiExperiment )
{
}

void SIG_ExperimentView::putIntoExperiment() {
  // slotHistory and slotAutosaveChanged reach this function without going
  // through putAllIntoExperiment, so it needs its own check. The LCD read
  // below is display-only and sits ahead of it.

  // update the generations display + progress-bar
  lcdnumberGenerations->display(theExperiment.population.getPoolGeneration());
  // generationProgBar

  if ( guiExperiment.experimentListView->isRunning() )
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
	 << "set title \"Maximum, minimum and average fitness values\"\n"
	 << "set xlabel 'Generation'\n"
	 << "set ylabel 'Fitness value'\n";

  stream << "plot '-' title 'Maximum fitness', '-' title 'Minimum fitness', '-' title 'Average fitness'\n";

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
      QMessageBox::information( this, "Info", "There is no evolution data to plot.");
      return;
    };

  QString fileName = QFileDialog::getSaveFileName( this, QString(), QString(), "Encapsulated PostScript Files (*.eps);;All Files (*)" );

  if (fileName.isNull())
    return;

  FILE *gnuPlotStdInPipe = popen( "gnuplot -persist -", "w" );

  if (gnuPlotStdInPipe==nullptr)
    {
      QMessageBox::warning( this, "Error", "Couldn't start gnuplot.");
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
};

void SIG_ExperimentView::slotShowFitnesscurve() {
  if (theExperiment.experimentHistory.isEmpty())
    {
      QMessageBox::information( this, "Info", "There is no evolution data to plot.");
      return;
    };

  //errno = 0;
  //signal(SIGPIPE,sigelSignalStandardHandler);
  FILE *gnuPlotStdInPipe = popen( "gnuplot -persist -", "w" );

  if (gnuPlotStdInPipe==nullptr ) {
    QMessageBox::warning( this, "Error", "Couldn't start gnuplot.");
    return;
  };

  QFile pipeFile;
 pipeFile.open( gnuPlotStdInPipe, QIODevice::WriteOnly );

  QTextStream pipeStream( &pipeFile );

  streamToGnuPlot( pipeStream );

  pipeStream << "quit\n";

  pipeFile.close();

  pclose( gnuPlotStdInPipe );
};

void SIG_ExperimentView::slotHistory(bool selected) {
  putIntoExperiment();
};

void SIG_ExperimentView::slotAutosaveChanged(int value) {
  if (theExperiment.getPath()!="new") {
    lcdnumberAutosave->display(value);
    putIntoExperiment();
  }
  else {
    // this is because a new experiment has no path where it is saved
    // so no autosaving can be done
    sliderAutosave->setValue(0);
    QMessageBox::warning( this, "Error", "You have to save the experiment first.");
  }
};

}
