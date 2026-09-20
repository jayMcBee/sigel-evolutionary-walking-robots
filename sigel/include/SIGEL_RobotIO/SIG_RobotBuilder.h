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
#ifndef SIGEL_ROBOTIO_SIG_ROBOTBUILDER_H
#define SIGEL_ROBOTIO_SIG_ROBOTBUILDER_H

namespace SIGEL_RobotIO { class SIG_RobotBuilder; }

#include <qstring.h>
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_RobotIO/SIG_RobotFactory.h"
#include "SIGEL_RobotIO/SIG_RobotScanner.h"
#include "SIGEL_RobotIO/SIG_RobotCompilerObjects.h"
#include "SIGEL_RobotIO/SIG_RobotCompilerStructure.h"

/**
 * This namespace contains all classes involved in reading in
 * and writing out SIG_Robot objects in the different supported
 * formats (modelling format / transfer format).
 */
namespace SIGEL_RobotIO {
        class SIG_RobotBuilder : public SIG_RobotFactory {
        private:
                QString filename;
        public:
                SIG_RobotBuilder (QString file);
                virtual ~SIG_RobotBuilder (void);

                SIG_Robot *build (void);
                void buildInto (SIG_Robot & rob);
        protected:
                QString loadFile (QString name);
                void firstPass (void);
                void secondPass (void);
        };
}

#endif
