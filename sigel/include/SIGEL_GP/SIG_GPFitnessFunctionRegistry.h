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
