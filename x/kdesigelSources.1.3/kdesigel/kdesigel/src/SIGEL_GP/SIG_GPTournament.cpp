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
#include "SIGEL_GP/SIG_GPTournament.h"

SIGEL_GP::SIG_GPTournament::SIG_GPTournament(SIGEL_Tools::SIG_Randomizer& randomizer,
					     SIG_GPFitnessTrainer& trainer, 
					     SIG_GPPopulation& actPool,
					     SIG_GPParameter& gpParameter,
					     SIGEL_Robot::SIG_LanguageParameters &languageP)
:gpPool(actPool),
trainer(trainer),
indis(),
justWaiting(false),
depNumber(0),
gpParameter(gpParameter),
randomizer(randomizer),
languageP(languageP)
{   
};

SIGEL_GP::SIG_GPTournament::~SIG_GPTournament()
{
  // This class owns the tournament individuals its subclasses build.
  // deleteContents() was the free: no setAutoDelete on this container, so
  // ~Q2PtrVector freed nothing and this call was the whole ownership.
  qDeleteAll( indis );
  indis.clear();
};

bool SIGEL_GP::SIG_GPTournament::run()
{ return true; };

bool SIGEL_GP::SIG_GPTournament::run(MT_Classifier *MetaClassifier)
{ return true; };

bool SIGEL_GP::SIG_GPTournament::classify(MT_Classifier *MetaClassifier)
{ return true; };


void SIGEL_GP::SIG_GPTournament::inhume( SIG_GPIndividual &corps )
{
  if (gpParameter.getLiveUndead())
    {
      QDir graveYardDir = gpParameter.getGraveYardDirectory();

      QString corpsFileName =   graveYardDir.path()
                              + QString( "/" )
                              + QString( "Individual_" )
                              + corps.getName()
                              + QString( "_Fitness_" )
                              + QString::number( corps.getFitness() )
                              + ".ind";

#ifdef SIG_DEBUG
      SIGEL_Tools::SIG_IO::cerr << "SIG_GPTournament saving dead Individual "
				<< corps.getName()
				<< "\n";
#endif

      QFile corpsFile( corpsFileName );

      if (corpsFile.open( QIODeviceBase::WriteOnly ))
	{
	  QTextStream buffer( &corpsFile );
	  corps.writeToFile( buffer,true );
	  corpsFile.close();
	}
      else
	SIGEL_Tools::SIG_IO::cerr << "Could not save corps under "
				  << corpsFileName
				  << "!\n";
    };
};

