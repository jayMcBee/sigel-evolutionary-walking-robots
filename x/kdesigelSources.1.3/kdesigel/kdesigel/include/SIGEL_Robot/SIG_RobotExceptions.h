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
#ifndef SIGEL_ROBOT_SIG_ROBOTEXCEPTIONS_H
#define SIGEL_ROBOT_SIG_ROBOTEXCEPTIONS_H

#include <qstring.h>
#include "SIGEL_Tools/SIG_Exception.h"

/* No comments today - too urgent. */

using SIGEL_Tools::SIG_Exception;

namespace SIGEL_Robot {
        /*
        class SIG_MultipleRootsError : public SIG_Exception {
	public:
                SIG_MultipleRootsError (QString file,
                                        int line,
                                        QString olderone,
                                        QString newerone)
                        : SIG_Exception (file, line,
                                         "\"" + newerone +
                                         "\" redefines previous root link \"" +
                                         olderone + "\".")
                        { }
        };
        */

        class SIG_UnstreamingError : public SIG_Exception {
        public:
                SIG_UnstreamingError (QString file, int line,
                                      QString message)
                        : SIG_Exception (file, line,
                                         "Unstreaming error: " + message + ".")
                        { }
        };

        class SIG_CannotMirtich : public SIG_Exception {
	public:
                SIG_CannotMirtich (QString file,
                                   int line,
                                   QString dxf)
                        : SIG_Exception (file, line,
                                         "Cannot calculate Mirtich's properties for \"" + dxf + "\".")
                        { }
        };

        class SIG_InitialLocationError : public SIG_Exception {
        public:
                SIG_InitialLocationError (QString file,
                                          int line,
                                          QString message)
                        : SIG_Exception (file, line,
                                         "Cannot determine initial location: " + message)
                        { }
        };

        class SIG_InvalidKinematicsError : public SIG_Exception {
	public:
                SIG_InvalidKinematicsError (QString file,
                                            int line,
                                            QString linkname)
                        : SIG_Exception (file, line,
                                         "Ambiguous initial values for \"" +
                                         linkname + "\".")
                        { }
        };
}

#endif
