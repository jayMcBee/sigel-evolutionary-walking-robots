/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler
  Copyright 2026 Jan Barnholt (SIGEL 2.0)

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
#ifndef SIGEL_GP_SIG_GPFITNESSFUNCTIONREGISTRY_H
#define SIGEL_GP_SIG_GPFITNESSFUNCTIONREGISTRY_H

#include "SIGEL_GP/SIG_GPFitnessFunction.h"

#include <QList>

#include <optional>

namespace SIGEL_GP
{

  // The one list of fitness functions. The registry owns the objects.
  class SIG_GPFitnessFunctionRegistry
  {
  public:
    static const QList<const SIG_GPFitnessFunction *> &fitnessFunctions();

    // Position in fitnessFunctions(); empty for an unknown ID.
    static std::optional<int> indexOf( const QString &serializedId );
  };

}

#endif // SIGEL_GP_SIG_GPFITNESSFUNCTIONREGISTRY_H
