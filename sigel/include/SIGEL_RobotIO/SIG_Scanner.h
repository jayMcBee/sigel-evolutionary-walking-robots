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
#ifndef SIGEL_ROBOTIO_SIG_SCANNER_H
#define SIGEL_ROBOTIO_SIG_SCANNER_H

namespace SIGEL_RobotIO { class SIG_Scanner; }

#include <qstring.h>

namespace SIGEL_RobotIO {
        namespace Symbol {
                extern int const None;
                extern int const EOS;
        }

        class SIG_Scanner {
        protected:
                QString text;
                int position, lineposition;
                QString currentSymbol;
                int currentSymType;
        public:
                SIG_Scanner (QString liesdas);
                virtual ~SIG_Scanner (void);

                void skipWhiteSpace (void);
                virtual void nextSymbol (void) = 0;
                void peekSymbol (int & symType, QString & symbol);
                void readSymbol (int & symType, QString & symbol);
                int currentLine (void) const;
        };
}

#endif
