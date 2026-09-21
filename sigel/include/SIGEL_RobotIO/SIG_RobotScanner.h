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
#ifndef SIGEL_ROBOTIO_SIG_ROBOTSCANNER_H
#define SIGEL_ROBOTIO_SIG_ROBOTSCANNER_H

namespace SIGEL_RobotIO { class SIG_RobotScanner; }

#include <qstring.h>
#include "SIGEL_RobotIO/SIG_Scanner.h"

namespace SIGEL_RobotIO {
        namespace RobotSymbol {
                extern int const openingBrace;
                extern int const closingBrace;
                extern int const openingParen;
                extern int const closingParen;
                extern int const word;
                extern int const number;
                extern int const string;
                extern int const semicolon;
                extern int const comma;
                extern int const equals;
                extern int const slash;
        }
        
        class SIG_RobotScanner : public SIG_Scanner {
        public:
                SIG_RobotScanner (QString sourceText);
                virtual ~SIG_RobotScanner (void);
                virtual void nextSymbol (void);
        };
}

#endif
