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
#include "SIGEL_GP/SIG_GPExperimentHistoryEntry.h"

#include <qtextstream.h>

namespace SIGEL_GP
{

  SIG_GPExperimentHistoryEntry::SIG_GPExperimentHistoryEntry( int generationNo,
							      QDateTime breakTime,
							      double maxFitness,
							      double minFitness,
							      double averageFitness )
    : generationNo( generationNo ),
      breakTime( breakTime ),
      maxFitness( maxFitness ),
      minFitness( minFitness ),
      averageFitness( averageFitness )
  { };

  SIG_GPExperimentHistoryEntry::SIG_GPExperimentHistoryEntry( QString entryString )
  {
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;

    QTextStream entryStream( &entryString, IO_ReadOnly );

    entryStream >> generationNo
		>> year
		>> month
		>> day
		>> hour
		>> minute
		>> second
		>> maxFitness
		>> minFitness
		>> averageFitness;

    QDate date( year, month, day );

    QTime time( hour, minute, second );

    breakTime.setDate( date );
    breakTime.setTime( time );
  };

  int SIG_GPExperimentHistoryEntry::getGenerationNo() const
  {
    return generationNo;
  };

  QDateTime SIG_GPExperimentHistoryEntry::getBreakTime() const
  {
    return breakTime;
  };

  double SIG_GPExperimentHistoryEntry::getMaxFitness() const
  {
    return maxFitness;
  };

  QString SIG_GPExperimentHistoryEntry::print() const
  {
    QString result;
    QTextStream resultStream( &result, IO_WriteOnly );

    resultStream << generationNo
		 << " "
		 << breakTime.date().year()
		 << " "
		 << breakTime.date().month()
		 << " "
		 << breakTime.date().day()
		 << " "
		 << breakTime.time().hour()
		 << " "
		 << breakTime.time().minute()
		 << " "
		 << breakTime.time().second()
		 << " "
		 << maxFitness
		 << " "
		 << minFitness
		 << " "
		 << averageFitness
		 << "\n";

    return result;
  };

  double SIG_GPExperimentHistoryEntry::getMinFitness() const
  {
    return minFitness;
  };

  double SIG_GPExperimentHistoryEntry::getAverageFitness() const
  {
    return averageFitness;
  };

}
