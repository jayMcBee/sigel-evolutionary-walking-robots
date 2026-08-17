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
#ifndef SIGEL_SIMULATION_SIG_DYNASYTEMWRONGNUMBEREXCEPTION_H
#define SIGEL_SIMULATION_SIG_DYNASYTEMWRONGNUMBEREXCEPTION_H

#include "SIGEL_Tools/SIG_Exception.h"

namespace SIGEL_Simulation
{

  /**
   * The SIG_DynaSystem throws a SIG_DynaSystemWrongNumberException if someone
   * tries to get a DynaObject by Number which doesn't exist.
   */
  class SIG_DynaSystemWrongNumberException : public SIGEL_Tools::SIG_Exception
    { 
    public:

      /**
       * Calls the superclasses SIG_Exception constructor with
       * this parameter list and prepends
       * "SIG_DynaSystemWrongNumberException: " to the message.
       */
      SIG_DynaSystemWrongNumberException(QString fileName,
					 int line,
					 QString message);

      /**
       * Calls the superclasses SIG_Exception constructor with
       * this parameter list and prepends
       * "SIG_DynaSystemWrongNumberException: " to the message.
       */
      SIG_DynaSystemWrongNumberException(QString fileName,
					 int line,
					 QString message,
					 SIGEL_Tools::SIG_Exception const& prevException);

    };

}

#endif // SIGEL_SIMULATION_SIG_DYNASYTEMWRONGNUMBEREXCEPTION_H
