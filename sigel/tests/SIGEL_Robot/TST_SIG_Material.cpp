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
#include "SIGEL_Robot/TST_SIG_Material.h"

#include "SIGEL_Robot/SIG_Material.h"
#include "SIGEL_Robot/SIG_Robot.h"

#include <QtTest>

void SIGEL_Robot::TST_SIG_Material::aPairNotSetReadsTheDefaultFriction()
{
  SIG_Robot robot;
  SIG_Material a( &robot, "a" );
  SIG_Material b( &robot, "b" );
  SIG_Material c( &robot, "c" );

  a.setFrictionValue( &b, 0.25 );

  // 0.6 is the friction of a pair that is not set.
  QCOMPARE( a.getFrictionValue( &c ), 0.6 );
  QCOMPARE( c.getFrictionValue( &a ), 0.6 );
  QCOMPARE( b.getFrictionValue( &c ), 0.6 );
}

void SIGEL_Robot::TST_SIG_Material::setFrictionValueSetsAndUpdatesBothPartners()
{
  SIG_Robot robot;
  SIG_Material a( &robot, "a" );
  SIG_Material b( &robot, "b" );
  SIG_Material c( &robot, "c" );

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

void SIGEL_Robot::TST_SIG_Material::anUpdateFromThePartnersSideReachesThisMaterial()
{
  SIG_Robot robot;
  SIG_Material a( &robot, "a" );
  SIG_Material b( &robot, "b" );
  a.setFrictionValue( &b, 0.25 );

  b.setFrictionValue( &a, 0.75 );

  QCOMPARE( a.getFrictionValue( &b ), 0.75 );

  // "Material a <elasticity> <density> <number of partners> b 0.75 <colour>"
  QString written;
  QTextStream stream( &written );
  a.writeToFileTransfer( stream );
  QCOMPARE( written.split( ' ' ).value( 4 ), QString( "1" ) );
  QVERIFY( written.contains( "b 0.75" ) );
}

void SIGEL_Robot::TST_SIG_Material::setFrictionValueOnItselfKeepsOnePartner()
{
  SIG_Robot robot;
  SIG_Material a( &robot, "a" );

  a.setFrictionValue( &a, 0.25 );
  a.setFrictionValue( &a, 0.75 );
  QCOMPARE( a.getFrictionValue( &a ), 0.75 );

  // "Material a <elasticity> <density> <number of partners> a 0.75 <colour>"
  QString written;
  QTextStream stream( &written );
  a.writeToFileTransfer( stream );
  QCOMPARE( written.split( ' ' ).value( 4 ), QString( "1" ) );
  QVERIFY( written.contains( "a 0.75" ) );
}

void SIGEL_Robot::TST_SIG_Material::constructorWithANameGivesTheDefaultMaterial()
{
  SIG_Robot robot;
  SIG_Material a( &robot, "steel" );

  QCOMPARE( a.getName(), QString( "steel" ) );

  // A robot file may leave out elasticity and colour: the material then has elasticity 1 and is white.
  QCOMPARE( a.getElasticity(), 1.0 );
  SIG_Vector colour = a.getColour();
  QCOMPARE( colour.x, 1.0 );
  QCOMPARE( colour.y, 1.0 );
  QCOMPARE( colour.z, 1.0 );

  // "Material steel <elasticity> <density> <number of partners> <colour>"
  QString written;
  QTextStream stream( &written );
  a.writeToFileTransfer( stream );
  QCOMPARE( written.split( ' ' ).value( 4 ), QString( "0" ) );
}

void SIGEL_Robot::TST_SIG_Material::eachSetterChangesOnlyItsOwnValue()
{
  SIG_Robot robot;
  SIG_Material a( &robot, "steel" );

  a.setDensity( 7850 );
  SIG_Vector colour = a.getColour();
  QCOMPARE( a.getDensity(), 7850.0 );
  QCOMPARE( a.getElasticity(), 1.0 );
  QCOMPARE( colour.x, 1.0 );
  QCOMPARE( colour.y, 1.0 );
  QCOMPARE( colour.z, 1.0 );
  QCOMPARE( a.getName(), QString( "steel" ) );

  a.setElasticity( 0.5 );
  colour = a.getColour();
  QCOMPARE( a.getElasticity(), 0.5 );
  QCOMPARE( a.getDensity(), 7850.0 );
  QCOMPARE( colour.x, 1.0 );
  QCOMPARE( colour.y, 1.0 );
  QCOMPARE( colour.z, 1.0 );
  QCOMPARE( a.getName(), QString( "steel" ) );

  a.setColour( SIG_Vector( 0.25, 0.5, 0.75 ) );
  colour = a.getColour();
  QCOMPARE( colour.x, 0.25 );
  QCOMPARE( colour.y, 0.5 );
  QCOMPARE( colour.z, 0.75 );
  QCOMPARE( a.getDensity(), 7850.0 );
  QCOMPARE( a.getElasticity(), 0.5 );
  QCOMPARE( a.getName(), QString( "steel" ) );
}

void SIGEL_Robot::TST_SIG_Material::writeToFileTransferWritesTheWholeRecord()
{
  SIG_Robot robot;
  SIG_Material steel( &robot, "steel" );
  SIG_Material rubber( &robot, "rubber" );
  steel.setElasticity( 0.5 );
  steel.setDensity( 7850 );
  steel.setColour( SIG_Vector( 0.25, 0.5, 0.75 ) );
  steel.setFrictionValue( &rubber, 0.8 );
  QString text;
  QTextStream stream( &text );

  steel.writeToFileTransfer( stream );

  // The keyword Material, then name, elasticity, density, number of partners, each partner with its value, colour.
  QCOMPARE( text, QString( "Material steel 0.5 7850 1 rubber 0.8 0.25 0.5 0.75 \n" ) );
}

void SIGEL_Robot::TST_SIG_Material::streamConstructorReadsARecordWithoutPartners()
{
  SIG_Robot robot;
  QString text = "plain 0.4 2700 0 0.25 0.5 0.75 Link";
  QTextStream in( &text, QIODevice::ReadOnly );

  SIG_Material read( &robot, in );
  QString next;
  in >> next;

  QCOMPARE( read.getName(), QString( "plain" ) );
  QCOMPARE( read.getElasticity(), 0.4 );
  QCOMPARE( read.getDensity(), 2700.0 );
  SIG_Vector colour = read.getColour();
  QCOMPARE( colour.x, 0.25 );
  QCOMPARE( colour.y, 0.5 );
  QCOMPARE( colour.z, 0.75 );
  QCOMPARE( next, QString( "Link" ) );

  // "Material plain <elasticity> <density> <number of partners> <colour>"
  QString written;
  QTextStream stream( &written );
  read.writeToFileTransfer( stream );
  QCOMPARE( written.split( ' ' ).value( 4 ), QString( "0" ) );
}

void SIGEL_Robot::TST_SIG_Material::streamConstructorPairsALoadedPartnerAndDropsOneNotYetLoaded()
{
  SIG_Robot robot;
  SIG_Material *known = new SIG_Material( &robot, "known" );
  robot.addMaterial( known );
  QString text = "steel 0.4 2700 2 notYetLoaded 0.9 known 0.35 0.25 0.5 0.75 Link";
  QTextStream in( &text, QIODevice::ReadOnly );

  SIG_Material *read = new SIG_Material( &robot, in );
  robot.addMaterial( read );
  QString next;
  in >> next;

  QCOMPARE( read->getFrictionValue( known ), 0.35 );
  QCOMPARE( known->getFrictionValue( read ), 0.35 );
  QCOMPARE( read->getElasticity(), 0.4 );
  QCOMPARE( read->getDensity(), 2700.0 );
  SIG_Vector colour = read->getColour();
  QCOMPARE( colour.x, 0.25 );
  QCOMPARE( colour.y, 0.5 );
  QCOMPARE( colour.z, 0.75 );
  QCOMPARE( next, QString( "Link" ) );

  // "Material steel <elasticity> <density> <number of partners> known 0.35 <colour>"
  QString written;
  QTextStream stream( &written );
  read->writeToFileTransfer( stream );
  QCOMPARE( written.split( ' ' ).value( 4 ), QString( "1" ) );
  QVERIFY( !written.contains( "notYetLoaded" ) );
}

void SIGEL_Robot::TST_SIG_Material::aPairSurvivesWritingAndReadingBothMaterials()
{
  SIG_Robot source;
  SIG_Material *a = new SIG_Material( &source, "a" );
  SIG_Material *b = new SIG_Material( &source, "b" );
  source.addMaterial( a );
  source.addMaterial( b );
  a->setFrictionValue( b, 0.25 );
  QString text;
  QTextStream out( &text );
  a->writeToFileTransfer( out );
  b->writeToFileTransfer( out );

  // The record of a names b before b is read; the record of b then sets the pair in both.
  SIG_Robot copy;
  QTextStream in( &text );
  QString keyword;
  in >> keyword;
  copy.addMaterial( new SIG_Material( &copy, in ) );
  in >> keyword;
  copy.addMaterial( new SIG_Material( &copy, in ) );
  SIG_Material *readA = copy.lookupMaterial( "a" );
  SIG_Material *readB = copy.lookupMaterial( "b" );

  QVERIFY( readA != nullptr );
  QVERIFY( readB != nullptr );
  QCOMPARE( readA->getFrictionValue( readB ), 0.25 );
  QCOMPARE( readB->getFrictionValue( readA ), 0.25 );

  // "Material a <elasticity> <density> <number of partners> b 0.25 <colour>"
  QString written;
  QTextStream stream( &written );
  readA->writeToFileTransfer( stream );
  QCOMPARE( written.split( ' ' ).value( 4 ), QString( "1" ) );
}

void SIGEL_Robot::TST_SIG_Material::aPairOfAMaterialWithItselfSurvivesWritingAndReading()
{
  SIG_Robot source;
  SIG_Material *steel = new SIG_Material( &source, "steel" );
  source.addMaterial( steel );
  steel->setFrictionValue( steel, 0.35 );
  QString text;
  QTextStream out( &text );
  steel->writeToFileTransfer( out );

  // The material being read is not yet in the copy, so the robot cannot find it by name.
  SIG_Robot copy;
  QTextStream in( &text );
  QString keyword;
  in >> keyword;
  SIG_Material *read = new SIG_Material( &copy, in );
  copy.addMaterial( read );

  QCOMPARE( read->getFrictionValue( read ), 0.35 );

  // "Material steel <elasticity> <density> <number of partners> steel 0.35 <colour>"
  QString written;
  QTextStream stream( &written );
  read->writeToFileTransfer( stream );
  QCOMPARE( written.split( ' ' ).value( 4 ), QString( "1" ) );
}
