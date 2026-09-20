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
#ifndef SIGEL_TOOLS_SIG_IO_H
#define SIGEL_TOOLS_SIG_IO_H

#include <cstdio>
#include <qtextstream.h>

/**
 * This namespace contains utility classes that could be needed
 * everywhere.
 */
namespace SIGEL_Tools
{

  /**
   * This class contains the three standard streams cin, cout and cerr
   * as QTextStreams.
   *
   * So they can be used together with th other
   * standard IO classes of QT.
   */
  class SIG_IO
    {
    public:
      /**
       * The standard in stream.
       */
      static QTextStream cin;

      /**
       * The standard out stream.
       */
      static QTextStream cout;

      /**
       * The standard error stream.
       */
      static QTextStream cerr;
    };

}

#endif // SIGEL_TOOLS_SIG_IO_H
