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
#ifndef SIGEL_ROBOT_SIG_LANGUAGEPARAMETERS_H
#define SIGEL_ROBOT_SIG_LANGUAGEPARAMETERS_H

namespace SIGEL_Robot { class SIG_LanguageParameters; }

#include "compat/q2compat.h"
#include <qtextstream.h>
#include <qstring.h>
#include "SIGEL_Robot/SIG_CommandParameters.h"

namespace SIGEL_Robot {
  /**
   * This class describes the possibilities and limitations
   * of the robot control language.
   * For each allowed command it contains an instance of
   * SIG_CommandParam describing the capabilities of the
   * command.
   */
        class SIG_LanguageParameters {
        public:
                /** One allowed command, with the name it is declared under. */
                struct NamedCommand { QString name; SIG_CommandParameters *value; };

        private:
                /**
                 * A dictionary of the commands allowed in the
                 * control language of the particular robot.
                 */
                // Phase D. Was Q2Dict. Its order rides inside every .exp and
                // every PVM transfer via writeToFileTransfer, so it is ordered,
                // and SIG_CommandParameters carries no name of its own.
                QList<NamedCommand> allowedCommands;
                /**
                 * The width of the registers of the robot's
                 * memory.
                 */
                int bitsPerRegister;

		/**
		 * The number of registers of the robot.
		 */
                int memSize;

		/**
		 *
		 */
		int maximalDelayTime;

        public:
                /**
                 * Standard constructor. Empty dictionary and
                 * 32-bit-registers.
                 */
                SIG_LanguageParameters (void);
                /**
                 * Reads the contents of the object from a stream,
                 * whose data may be originating from a file or
                 * from a network.
                 */
                SIG_LanguageParameters (QTextStream & tx, bool nir = false);
                /**
                 * Destructor.
                 */
                ~SIG_LanguageParameters (void);

                /**
                 * Adds a command to the dictionary.
                 */
                void addCommand (QString name, SIG_CommandParameters *cmdP);

		/**
		 * Removes a command from the dictionary.
		 */
		void removeCommand( QString name );

                /**
                 * Query whether a command is allowed.
                 */
                bool hasCommand (QString name) const;
                /**
                 * Getting the object describing the command properties.
                 */
                SIG_CommandParameters *getCommand (QString name) const;

                /**
                 * Sets the width of the memory cells
                 * within the robots control unit.
                 */
                void setRegisterWidth (int width);
                /**
                 * Provides the width of the robot's memory cells.
                 */
                int getRegisterWidth (void) const;

                /**
                 * This sets the number of registers.
                 */
                void setMemorySize (int amount);

                /**
                 * How many registers are there?
                 */
                int getMemorySize (void) const;

		/**
		 * Sets the maximal time that a delay may take place.
		 */
		void setMaximalDelayTime( int amount );

		/**
		 * Gets the maximal time that a delay may take place.
		 */
		int getMaximalDelayTime( void ) const;

                /**
                 * Writes the object's data to a stream. The stream
                 * may be connected to a file or to a network or
                 * to whatever.
                 */
                void writeToFileTransfer (QTextStream & tx) const;
        };
}

#endif
