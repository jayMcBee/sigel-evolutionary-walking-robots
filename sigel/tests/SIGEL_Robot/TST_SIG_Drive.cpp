/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler
  Copyright 2026 Jan Barnholt (SIGEL 2.0)

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
#include "SIGEL_Robot/TST_SIG_Drive.h"

#include "SIGEL_Robot/SIG_Drive.h"
#include "SIGEL_Robot/SIG_GlueJoint.h"
#include "SIGEL_Robot/SIG_Robot.h"
#include "SIGEL_Robot/SIG_RobotExceptions.h"

#include <QtTest>

void SIGEL_Robot::TST_SIG_Drive::unknownModeWordThrows()
{
  SIG_Robot robot;
  robot.addJoint( new SIG_GlueJoint( &robot, "joint", 0 ) );

  // "<mode> <name> <number> <minimal force> <maximal force> <joint>"
  QString knownText = "force drive 0 0 1 joint ";
  QTextStream knownStream( &knownText, QIODevice::ReadOnly );
  SIG_Drive known( &robot, knownStream );
  QCOMPARE( known.getMode(), SIG_Drive::tForceMode );

  QString unknownText = "sideways drive 0 0 1 joint ";
  QTextStream unknownStream( &unknownText, QIODevice::ReadOnly );
  QVERIFY_THROWS_EXCEPTION( SIG_UnstreamingError, SIG_Drive unknown( &robot, unknownStream ) );
}
