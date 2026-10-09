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
#include "SIGEL_Robot/SIG_MaterialTest.h"

#include "SIGEL_Robot/SIG_Material.h"
#include "SIGEL_Robot/SIG_Robot.h"

#include <QtTest>

void SIGEL_Robot::SIG_MaterialTest::setFrictionValueSetsAndUpdatesBothPartners()
{
  SIG_Robot robot;
  SIG_Material a( &robot, "a" );
  SIG_Material b( &robot, "b" );
  SIG_Material c( &robot, "c" );

  // The value for a pair that is not set.
  QCOMPARE( a.getFrictionValue( &b ), 0.6 );

  a.setFrictionValue( &b, 0.25 );
  QCOMPARE( a.getFrictionValue( &b ), 0.25 );
  QCOMPARE( b.getFrictionValue( &a ), 0.25 );

  a.setFrictionValue( &c, 0.5 );
  QCOMPARE( a.getFrictionValue( &b ), 0.25 );
  QCOMPARE( a.getFrictionValue( &c ), 0.5 );

  a.setFrictionValue( &b, 0.75 );
  QCOMPARE( a.getFrictionValue( &b ), 0.75 );
  QCOMPARE( a.getFrictionValue( &c ), 0.5 );
  QCOMPARE( b.getFrictionValue( &a ), 0.75 );

  // The written record gives the number of partners: an update must not add one.
  // "Material a <elasticity> <density> <number of partners> b 0.75 c 0.5 <colour>"
  QString written;
  QTextStream stream( &written );
  a.writeToFileTransfer( stream );
  QCOMPARE( written.split( ' ' ).value( 4 ), QString( "2" ) );
  QVERIFY( written.contains( "b 0.75" ) );
  QVERIFY( written.contains( "c 0.5" ) );
}

void SIGEL_Robot::SIG_MaterialTest::materialReadFromStreamKeepsOnlyALoadedPartner()
{
  SIG_Robot robot;
  SIG_Material *known = new SIG_Material( &robot, "known" );
  robot.addMaterial( known );

  QString laterText = "later 1 1 1 known 0.25 0 0 0 ";
  QTextStream laterStream( &laterText, QIODevice::ReadOnly );
  SIG_Material *later = new SIG_Material( &robot, laterStream );
  robot.addMaterial( later );

  QCOMPARE( later->getFrictionValue( known ), 0.25 );
  QCOMPARE( known->getFrictionValue( later ), 0.25 );

  QString earlyText = "early 1 1 1 notYetLoaded 0.9 0 0 0 ";
  QTextStream earlyStream( &earlyText, QIODevice::ReadOnly );
  SIG_Material *early = new SIG_Material( &robot, earlyStream );
  robot.addMaterial( early );

  // 0.6 is the value for a pair that is not set: the pair is dropped.
  QCOMPARE( early->getFrictionValue( known ), 0.6 );
}
