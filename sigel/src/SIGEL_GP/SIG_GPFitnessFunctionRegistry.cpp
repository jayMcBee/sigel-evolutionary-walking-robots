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
#include "SIGEL_GP/SIG_GPFitnessFunctionRegistry.h"

#include "SIGEL_GP/SIG_GPSimpleFitnessFunction.h"
#include "SIGEL_GP/SIG_GPRealSpeedFitnessFunction.h"
#include "SIGEL_GP/SIG_GPNiceWalkingFitnessFunction.h"
#include "SIGEL_GP/SIG_GPForceFitnessFunction.h"

#ifdef SIG_INCLUDE_ZORC_SUPPORT
#include "SIGEL_GP/SIG_GPAdaptiveWalkingFitnessFunction.h"
#include "SIGEL_GP/SIG_GPRemoteZORCFitnessFunction.h"
#endif

#include <algorithm>
#include <iterator>

namespace SIGEL_GP
{

  const QList<const SIG_GPFitnessFunction *> &SIG_GPFitnessFunctionRegistry::fitnessFunctions()
  {
    static const SIG_GPSimpleFitnessFunction simple;
    static const SIG_GPRealSpeedFitnessFunction realSpeed;
    static const SIG_GPNiceWalkingFitnessFunction niceWalking;
    static const SIG_GPForceFitnessFunction force;

#ifdef SIG_INCLUDE_ZORC_SUPPORT
    static const SIG_GPAdaptiveWalkingFitnessFunction adaptiveZorcWalking;
    static const SIG_GPRemoteZORCFitnessFunction remoteZORC;
#endif

    static const QList<const SIG_GPFitnessFunction *> functions = {
      &simple,
      &realSpeed,
      &niceWalking,
      &force,

#ifdef SIG_INCLUDE_ZORC_SUPPORT
      &adaptiveZorcWalking,
      &remoteZORC
#endif
    };
    return functions;
  }

  std::optional<int> SIG_GPFitnessFunctionRegistry::indexOf( const QString &serializedId )
  {
    const QList<const SIG_GPFitnessFunction *> &functions = fitnessFunctions();

    const auto found = std::find_if( functions.begin(), functions.end(),
                                     [&serializedId]( const SIG_GPFitnessFunction *function )
                                     { return function->serializedId() == serializedId; } );

    if( found == functions.end() )
      return std::nullopt;

    return int( std::distance( functions.begin(), found ) );
  }

}
