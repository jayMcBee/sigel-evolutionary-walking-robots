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
#include "SIGEL_Robot/TST_SIG_Robot.h"

#include "SIGEL_Robot/SIG_Body.h"
#include "SIGEL_Robot/SIG_ContactSensor.h"
#include "SIGEL_Robot/SIG_Drive.h"
#include "SIGEL_Robot/SIG_GlueJoint.h"
#include "SIGEL_Robot/SIG_Link.h"
#include "SIGEL_Robot/SIG_Material.h"
#include "SIGEL_Robot/SIG_Robot.h"

#include <QtTest>

void SIGEL_Robot::TST_SIG_Robot::lookupFindsEachKindOfPartByName()
{
  SIG_Robot robot;
  SIG_Body *body = new SIG_Body( &robot, "body", "d" );
  SIG_Material *material = new SIG_Material( &robot, "material" );
  SIG_Link *link = new SIG_Link( &robot, "link", 0 );
  SIG_Joint *joint = new SIG_GlueJoint( &robot, "joint", 0 );
  SIG_Drive *drive = new SIG_Drive( &robot, "drive", 0 );
  SIG_Sensor *sensor = new SIG_ContactSensor( &robot, "sensor", 0 );

  // SIG_Robot owns the parts and frees them.
  robot.addBody( body );
  robot.addMaterial( material );
  robot.addLink( link );
  robot.addJoint( joint );
  robot.addDrive( drive );
  robot.addSensor( sensor );

  QCOMPARE( robot.lookupBody( "body" ), body );
  QCOMPARE( robot.lookupMaterial( "material" ), material );
  QCOMPARE( robot.lookupLink( "link" ), link );
  QCOMPARE( robot.lookupJoint( "joint" ), joint );
  QCOMPARE( robot.lookupDrive( "drive" ), drive );
  QCOMPARE( robot.lookupSensor( "sensor" ), sensor );
  QCOMPARE( robot.lookupLink( "MISSING" ), nullptr );
}
