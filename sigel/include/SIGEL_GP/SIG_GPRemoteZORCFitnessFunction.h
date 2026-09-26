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
#ifndef SIGEL_GP_SIG_GPRemoteZORCFitnessFunction_H
#define SIGEL_GP_SIG_GPRemoteZORCFitnessFunction_H

#include "SIGEL_GP/SIG_GPFitnessFunction.h"


#include <fcntl.h>
#include <unistd.h>
#include <termios.h>


namespace SIGEL_GP
{

 /**
   * The 'SIG_GPRemoteZORCFitnessFunction' is a special fitness function
   * that can be used only in conjunction with the ZORC robot connected to
   * the machine via serial interface.
   *
   * Normally one wouldn't use this fitness function in ordinary experiments.
   */
   class SIG_GPRemoteZORCFitnessFunction : public SIG_GPFitnessFunction
   {
    public:

    /**
      * The desctructor.
      */
      ~SIG_GPRemoteZORCFitnessFunction();

    /**
      * Evaluate fitness of the current program.
      * This central method is transferring the current program to ZORC over
      * the serial line and computes the fitness based on the distance travelled
      * by the ZORC robot while executing the program.
		*
		* To be on the safe side the connection to ZORC is checked using the RS232-Ping
		* facility of ZORC, thus preventing the slave from hanging if nothing is
		* actually connected. Lot's of additional checks should help to locate errors
		* when something else goes wrong.
      */
      double   evalFitness( SIGEL_Program::SIG_Program &program,
                            SIGEL_Robot::SIG_Robot &rob,
                            SIGEL_Environment::SIG_Environment &environment,
                            SIGEL_Simulation::SIG_SimulationParameters &simparameter );

      QString  serializedId() const { return "RemoteZORCFitnessFunction"; }

    /**
      * This method sends all data contained in the QString object over the
      * serial interface specified by the 'serIF' file descriptor.
      * The method returns when all data has been sent and the serial buffer is empty.
      */
      void     sendOverSerialLine(int serIF, const QString &txtToSend);

    /**
      * Set some parameters of the serial device for proper operation with ZORC.
      * Returns -1 in case any error occurred.
      */
      int      SetSerial(int fd, long baud, int handshake);

    /**
      * Wait for input on the specified serial device for 'timeOutSecs' number of seconds.
      * Returns 'true' when data is pending, 'false' otherwise.
      */
      bool     timedSerialWait(int serIF, int timeOutSecs);

	 /**
     * This method goes one step into the ZORC menu hierarchy.
     */
		void 		goZORCMenu(int serIF, char inChoice);

   };

}

#endif // SIGEL_GP_SIG_GPRemoteZORCFitnessFunction_H
