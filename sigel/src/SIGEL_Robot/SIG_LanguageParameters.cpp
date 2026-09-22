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
#include "SIGEL_Robot/SIG_LanguageParameters.h"
#include "SIGEL_Robot/SIG_CommandParameters.h"
#include "SIGEL_Robot/SIG_RobotExceptions.h"

namespace SIGEL_Robot {
        SIG_LanguageParameters::SIG_LanguageParameters (void)
                : bitsPerRegister (32),
                  memSize (8),
		  maximalDelayTime(5000),
                  allowedCommands ()
        {
	  // ORDER MATTERS AND IS NOT ALPHABETICAL. A default-constructed object
	  // writes this order, and it must match what a loaded file writes.
	  SIG_CommandParameters *commandParameters = new SIG_CommandParameters();
	  commandParameters->setDuration( 0.001 );
	  this->addCommand( "MUL", commandParameters );

	  commandParameters = new SIG_CommandParameters();
	  commandParameters->setDuration( 0.001 );
	  this->addCommand( "MOVE", commandParameters );

	  commandParameters = new SIG_CommandParameters();
	  commandParameters->setDuration( 0.001 );
	  this->addCommand( "CMP", commandParameters );

	  commandParameters = new SIG_CommandParameters();
	  commandParameters->setDuration( 0.001 );
	  this->addCommand( "COPY", commandParameters );

	  commandParameters = new SIG_CommandParameters();
	  commandParameters->setDuration( 0.001 );
	  this->addCommand( "LOAD", commandParameters );

	  commandParameters = new SIG_CommandParameters();
	  commandParameters->setDuration( 0.001 );
	  this->addCommand( "JMP", commandParameters );

	  commandParameters = new SIG_CommandParameters();
	  commandParameters->setDuration( 0.001 );
	  this->addCommand( "SENSE", commandParameters );

	  commandParameters = new SIG_CommandParameters();
	  commandParameters->setDuration( 0.001 );
	  this->addCommand( "NOP", commandParameters );

	  commandParameters = new SIG_CommandParameters();
	  commandParameters->setDuration( 0.001 );
	  this->addCommand( "SUB", commandParameters );

	  commandParameters = new SIG_CommandParameters();
	  commandParameters->setDuration( 0.001 );
	  this->addCommand( "DIV", commandParameters );

	  commandParameters = new SIG_CommandParameters();
	  commandParameters->setDuration( 0.001 );
	  this->addCommand( "MIN", commandParameters );

	  commandParameters = new SIG_CommandParameters();
	  commandParameters->setDuration( 0.001 );
	  this->addCommand( "DELAY", commandParameters );

	  commandParameters = new SIG_CommandParameters();
	  commandParameters->setDuration( 0.001 );
	  this->addCommand( "ADD", commandParameters );

	  commandParameters = new SIG_CommandParameters();
	  commandParameters->setDuration( 0.001 );
	  this->addCommand( "MOD", commandParameters );

	  commandParameters = new SIG_CommandParameters();
	  commandParameters->setDuration( 0.001 );
	  this->addCommand( "MAX", commandParameters );

	}

        SIG_LanguageParameters::SIG_LanguageParameters (QTextStream & tx,
                                                        bool nir)
        { // nir = not in robot
                QString tmpstr;
                int commandCount;

                if (nir) {
			tx >> tmpstr;
			if (tmpstr != "LanguageParameters")
				throw SIG_UnstreamingError (__FILE__, __LINE__, "LanguageParameters expected");
		}

                // else "LanguageParameters" already read by SIG_Robot

                tx >> bitsPerRegister;
                tx >> memSize;
		tx >> maximalDelayTime;

                tx >> commandCount;
                for (int i = 0; i < commandCount; i++) {
                        tx >> tmpstr;
                        allowedCommands.append (NamedCommand{ tmpstr, new SIG_CommandParameters (tx) });
                }
        }

        SIG_LanguageParameters::~SIG_LanguageParameters (void)
        {
		// This class owns the SIG_CommandParameters it inserted.
		for (const NamedCommand &c : allowedCommands) delete c.value;
		allowedCommands.clear ();

	  
        }

        void SIG_LanguageParameters::addCommand (QString name, SIG_CommandParameters *cmdP)
        {
                allowedCommands.append (NamedCommand{ name, cmdP });
        }

        void SIG_LanguageParameters::removeCommand( QString name )
	{
	  // This class owns its commands, so removing one frees it. 2003 armed
	  // autoDelete in the constructor, which made QDict::remove() delete.
	  // Removes the LAST match, which is what Qt 2's newest-wins take() did.
	  for (qsizetype i = allowedCommands.size() - 1; i >= 0; --i)
	    if (allowedCommands.at(i).name == name) {
	      delete allowedCommands.at(i).value;
	      allowedCommands.removeAt(i);
	      return;
	    }
	}

        bool SIG_LanguageParameters::hasCommand (QString name) const
        {
                for (const NamedCommand &c : allowedCommands)
                        if (c.name == name) return true;
                return false;
        }
        
        SIG_CommandParameters *SIG_LanguageParameters::getCommand (QString name) const
        {
                // Last match wins, as Qt 2's QDict did.
                for (qsizetype i = allowedCommands.size() - 1; i >= 0; --i)
                        if (allowedCommands.at(i).name == name)
                                return allowedCommands.at(i).value;
                return 0;
        }

        void SIG_LanguageParameters::setRegisterWidth (int width)
        {
                bitsPerRegister = width;
        }

        int SIG_LanguageParameters::getRegisterWidth (void) const
        {
                return bitsPerRegister;
        }

        void SIG_LanguageParameters::setMemorySize (int amount)
        {
                memSize = amount;
        }

        int SIG_LanguageParameters::getMemorySize (void) const
        {
                return memSize;
        }

        void SIG_LanguageParameters::setMaximalDelayTime( int amount )
	{
	  maximalDelayTime = amount;
	}

        int SIG_LanguageParameters::getMaximalDelayTime( void ) const
	{
	  return maximalDelayTime;
	}

        const QList<SIG_LanguageParameters::NamedCommand> &
        SIG_LanguageParameters::getCommands (void) const
        {
                return allowedCommands;
        }

        void SIG_LanguageParameters::writeToFileTransfer (QTextStream & tx) const
        {
                tx << "LanguageParameters " << bitsPerRegister << ' ' << memSize << ' ' << maximalDelayTime << ' ';
                tx << allowedCommands.count () << '\n';
                for (const NamedCommand &c : allowedCommands) {
                        tx << c.name << ' ';
                        c.value->writeToFileTransfer (tx);
                }
        }
}
