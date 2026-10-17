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
