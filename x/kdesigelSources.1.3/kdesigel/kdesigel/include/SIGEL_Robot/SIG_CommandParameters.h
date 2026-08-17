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
#ifndef SIGEL_ROBOT_SIG_COMMANDPARAMETERS_H
#define SIGEL_ROBOT_SIG_COMMANDPARAMETERS_H

namespace SIGEL_Robot { class SIG_CommandParameters; }

#include <qdatetime.h>
#include <qtextstream.h>
#include <qstring.h>

namespace SIGEL_Robot {
        /**
         * This class contains some information about the
         * behaviour of commands of the robot control
         * language.
         */
        class SIG_CommandParameters {
        private:
                /**
                 * This is the amount of time, the commands
                 * needs to be executed properly. This is
                 * given in seconds.
                 * Of course this does not apply to adjustable
                 * delay commands.
                 */
                double duration;
        public:
                /**
                 * Standard constructor. Default values
                 * are a duration of 1 microsecond.
                 */
                SIG_CommandParameters (void);
                /**
                 * A constructor that reads the objects
                 * values from a stream. It is used when loading
                 * a compiled robot from file or when
                 * propagating a robot through the network.
                 */
                SIG_CommandParameters (QTextStream & tx);
                /**
                 * Destructor.
                 */
                ~SIG_CommandParameters (void);

                /**
                 * Sets the run time duration of the command.
                 */
                void setDuration (double duration);
                /**
                 * Returns the run time duration of the command.
                 */
                double getDuration (void) const;

                /**
                 * Streaming function. This will be used when saving
                 * to file or propagating through a network.
                 */
                void writeToFileTransfer (QTextStream & tx) const;
        };
}

#endif
