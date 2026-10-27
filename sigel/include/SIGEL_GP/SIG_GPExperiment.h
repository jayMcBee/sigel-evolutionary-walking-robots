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
#ifndef SIGEL_GP_SIG_GPEXPERIMENT_H
#define SIGEL_GP_SIG_GPEXPERIMENT_H

#include <memory>
#include "SIGEL_GP/SIG_GPSlaveExperiment.h"

#include "MT_Control/MT_Controller.h"

#include <qstring.h>
#include <qtextstream.h>

namespace SIGEL_GP
{

/**
 * SIG_GPExperiment is the main data structure of an evolution run: the data
 * of SIG_GPSlaveExperiment plus the controller of the MetaGP system.
 */

class SIG_GPExperiment : public SIG_GPSlaveExperiment {
 public:
  /**
   * Builds an empty experiment.
   * @param exp
   *  Is not used.
   */
  SIG_GPExperiment(QString exp);

  /**
   * Builds an empty experiment.
   */
  SIG_GPExperiment();

  /**
   * Reads the experiment from a text stream.
   * @param file
   *  The stream that holds a saved experiment.
   */
  void loadExperiment(QTextStream &file);

  /**
   * Writes the experiment to a text stream.
   * @param file
   *  The stream that receives the experiment.
   */
  void saveExperiment(QTextStream & file);

  /**
   * The controller of the MetaGP system. It is destroyed before the members
   * of SIG_GPSlaveExperiment.
   */
  std::unique_ptr< MT_Controller > mtController;
};

}

#endif // SIGEL_GP_SIG_GPEXPERIMENT_H
