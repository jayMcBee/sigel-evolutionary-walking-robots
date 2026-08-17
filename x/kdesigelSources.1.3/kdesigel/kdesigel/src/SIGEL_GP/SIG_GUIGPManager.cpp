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

#include <qapplication.h>
#include <qlcdnumber.h>

namespace SIGEL_GP
{

  SIG_GUIGPManager::SIG_GUIGPManager( SIGEL_MasterGUI::SIG_Experiment &guiExperiment )
    : SIG_GPManager( guiExperiment.gpExperiment ),
      guiExperiment( guiExperiment ),
      individualItems( guiExperiment.allIndividualsView->individualList->listviewIndividuals->childCount() )
  {
    QListView *listView = guiExperiment.allIndividualsView->individualList->listviewIndividuals;
    QListViewItemIterator listIter( listView );

    // update generations display (this line looks cool, doesn't it ?!)
   // guiExperiment.experimentView->lcdnumberGenerations->display(actExperiment.population.getPoolGeneration());

    while ( listIter.current() )
      {
	SIGEL_MasterGUI::SIG_IndividualListItem *actItem =
	  static_cast<SIGEL_MasterGUI::SIG_IndividualListItem*>( listIter.current() );
	individualItems.insert( actItem->poolPosition, actItem );
	++listIter;
      };
  };

  void SIG_GUIGPManager::haveABreak()
  {
    //    qApp->wakeUpGuiThread();
    qApp->processEvents( actExperiment.gpParameter.getPassiveTime() );
    //    msleep( actExperiment.gpParameter.getPassiveTime() );
  };

  void SIG_GUIGPManager::messageEvolutionStop()
  {
    schlussJetzt = true;
    //    guiExperiment.slotEvolutionStopped();
    //    QThread::exit();
  };

  void SIG_GUIGPManager::updateIndividualView( int poolPos )
  {
    SIG_GPIndividual &actInd = actExperiment.population.getIndividual( poolPos );

    individualItems[ poolPos ]->setTo( &actInd );

    // update generations display (this line looks cool, doesn't it ?!)
    //guiExperiment.experimentView->lcdnumberGenerations->display(actExperiment.population.getPoolGeneration());
  };

}
