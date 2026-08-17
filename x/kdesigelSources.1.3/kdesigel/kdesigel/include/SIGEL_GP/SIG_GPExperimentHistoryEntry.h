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
#ifndef SIGEL_GP_SIG_GPEXPERIMENTHISTORYENTRY_H
#define SIGEL_GP_SIG_GPEXPERIMENTHISTORYENTRY_H

#include <qdatetime.h>

namespace SIGEL_GP
{

class SIG_GPExperimentHistoryEntry
  {

  public:

    SIG_GPExperimentHistoryEntry( int generationNo,
				  QDateTime breakTime,
				  double maxFitness,
				  double minFitness,
				  double averageFitness );

    SIG_GPExperimentHistoryEntry( QString entryString );

    int getGenerationNo() const;

    QDateTime getBreakTime() const;

    double getMaxFitness() const;

    double getMinFitness() const;

    double getAverageFitness() const;

    QString print() const;

  private:

    int generationNo;

    QDateTime breakTime;

    double maxFitness;

    double minFitness;

    double averageFitness;

  };

}

#endif // SIGEL_GP_SIG_GPEXPERIMENTHISTORYENTRY_H
