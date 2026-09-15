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
#include "SIGEL_GP/SIG_GUIGPManager.h"
#include <QTreeWidget>
#include "SIGEL_MasterGUI/SIG_Experiment.h"
#include "SIGEL_MasterGUI/SIG_IndividualListItem.h"

#include <qapplication.h>
#include <qlcdnumber.h>

namespace SIGEL_GP
{

  SIG_GUIGPManager::SIG_GUIGPManager( SIGEL_MasterGUI::SIG_Experiment &guiExperiment )
    : SIG_GPManager( guiExperiment.gpExperiment ),
      guiExperiment( guiExperiment ),
      individualItems( guiExperiment.allIndividualsView->individualList->listviewIndividuals->topLevelItemCount() )
  {
    QTreeWidget *listView = guiExperiment.allIndividualsView->individualList->listviewIndividuals;
    QTreeWidgetItemIterator listIter( listView );

    // update generations display (this line looks cool, doesn't it ?!)
   // guiExperiment.experimentView->lcdnumberGenerations->display(actExperiment.population.getPoolGeneration());

    while ( *listIter )
      {
	SIGEL_MasterGUI::SIG_IndividualListItem *actItem =
	  static_cast<SIGEL_MasterGUI::SIG_IndividualListItem*>( (*listIter) );
	// insert(): a slot assignment. This container has no setAutoDelete, so
	// Qt 2 deleted nothing here -- the items belong to the list view.
	// Qt 2's QVector::insert took a uint, so a negative index wrapped huge and was
	// REJECTED. A signed test alone would let it through and index out of range.
	if (actItem->poolPosition >= 0
	    && actItem->poolPosition < individualItems.size())
	  individualItems[ actItem->poolPosition ] = actItem;
	++listIter;
      };
  };

  void SIG_GUIGPManager::haveABreak()
  {
    // The ONLY read of the `passive time' GP parameter in the tree, and it is
    // not what the name says: a millisecond budget for the event pump, not a
    // pause between generations. No pause is implemented anywhere.
    qApp->processEvents( QEventLoop::AllEvents,
                         actExperiment.gpParameter.getPassiveTime() );
  };

  void SIG_GUIGPManager::messageEvolutionStop()
  {
    // Sets the flag, stops nothing itself. slotEvolutionStopped() stays
    // commented out: SIG_Experiment calls it after start() returns, and
    // calling it here too would announce the run finished while it still runs.
    schlussJetzt = true;
    //    guiExperiment.slotEvolutionStopped();
  };

  void SIG_GUIGPManager::updateIndividualView( int poolPos )
  {
    SIG_GPIndividual &actInd = actExperiment.population.getIndividual( poolPos );

    // Qt 2's QVector::operator[] went through at(), which WARNED and then clamped
    // to element 0 -- so out of range, 1.3 updated the wrong item and said so.
    // QList::operator[] is UB there instead. This skips, which is neither: no
    // UB, and no wrong item. Keep the warning, or the failure becomes silent
    // and this is strictly less diagnosable than what it replaces.
    SIGEL_MasterGUI::SIG_IndividualListItem *item = individualItems.value( poolPos );
    if (item)
      item->setTo( &actInd );
    else
      qWarning( "SIG_GUIGPManager: no list item for pool position %d (size %lld)",
                poolPos, static_cast< long long >( individualItems.size() ) );

    // update generations display (this line looks cool, doesn't it ?!)
    //guiExperiment.experimentView->lcdnumberGenerations->display(actExperiment.population.getPoolGeneration());
  };

}
