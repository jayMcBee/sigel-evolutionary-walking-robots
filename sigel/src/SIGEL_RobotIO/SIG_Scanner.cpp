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
#include "SIGEL_RobotIO/SIG_Scanner.h"
#include <stdio.h>

namespace SIGEL_RobotIO {
        namespace Symbol {
                int const None = -1;
                int const EOS = -2;
        }
        
        SIG_Scanner::SIG_Scanner (QString liesdas)
                : text (liesdas),
                  position (0),
                  lineposition (1),
                  currentSymType (Symbol::None)
        { }

        SIG_Scanner::~SIG_Scanner (void)
        { }

        void SIG_Scanner::skipWhiteSpace (void)
        {
                bool commentdetected;
                do {
                        while ((position < text.length ()) &&
                               (text [position].isSpace ())) {
                                if (text [position] == '\n')
                                        lineposition++;
                                position++;
                        }
                        if ((position < text.length ()) &&
                            (text [position] == '#')) {
                                commentdetected = true;
                                while ((position < text.length ()) &&
                                       (text [position] != '\n'))
                                        position++;
                        } else
                                commentdetected = false;
                } while (commentdetected);
        }

        void SIG_Scanner::peekSymbol (int & symType, QString & symbol)
        {
                symType = currentSymType;
                symbol = currentSymbol;
        }

        void SIG_Scanner::readSymbol (int & symType, QString & symbol)
        {
                peekSymbol (symType, symbol);
                nextSymbol ();
        }

        int SIG_Scanner::currentLine (void) const
        {
                return lineposition;
        }
}
