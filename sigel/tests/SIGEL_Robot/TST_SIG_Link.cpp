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
#include "SIGEL_Robot/TST_SIG_Link.h"

#include "SIGEL_Robot/SIG_Body.h"
#include "SIGEL_Robot/SIG_Link.h"
#include "SIGEL_Robot/SIG_Material.h"
#include "SIGEL_Robot/SIG_RobotExceptions.h"
#include "SIGEL_Robot/SIG_Robot.h"

#include <QTextStream>
#include <QtTest>

#include <memory>

namespace {

  // The link that the unstreaming constructor reads from in; null, with what it
  // throws in thrown, if it throws.
  std::unique_ptr< SIGEL_Robot::SIG_Link > unstreamedLink( SIGEL_Robot::SIG_Robot &robot, QTextStream &in, QString &thrown )
  {
    try {
      return std::make_unique< SIGEL_Robot::SIG_Link >( &robot, in );
    }
    catch ( const SIGEL_Tools::SIG_Exception &e ) {
      thrown = e.getMessage();
    }
    catch ( ... ) {
      thrown = "an exception that is not a SIG_Exception";
    }
    return nullptr;
  }

}

// A robot file may declare a point of a link twice.
void SIGEL_Robot::TST_SIG_Link::pointAddedTwiceKeepsTheLastValue()
{
  SIG_Link link( 0, "L", 0 );

  link.addPoint( "P", SIG_Vector( 1, 0, 0 ) );
  link.addPoint( "P", SIG_Vector( 2, 0, 0 ) );

  QCOMPARE( link.getPoint( "P" ).x, 2.0 );
}

void SIGEL_Robot::TST_SIG_Link::addNoCollideRegistersThePairOnBothLinksOnce()
{
  SIG_Robot robot;
  SIG_Link first( &robot, "first", 0 );
  SIG_Link second( &robot, "second", 1 );

  first.addNoCollide( &second );

  QCOMPARE( first.getNoCollides().count(), 1 );
  QCOMPARE( second.getNoCollides().count(), 1 );
  QCOMPARE( first.getNoCollides().value( 0 ), &second );
  QCOMPARE( second.getNoCollides().value( 0 ), &first );

  first.addNoCollide( &second );

  QCOMPARE( first.getNoCollides().count(), 1 );
  QCOMPARE( second.getNoCollides().count(), 1 );
}

void SIGEL_Robot::TST_SIG_Link::writeToFileTransferQuotesABodyNameWithWhitespace()
{
  SIG_Robot robot;
  SIG_Body body( &robot, "left\tarm.wrl", "parts/" );
  SIG_Material material( &robot, "steel" );
  SIG_Link link( &robot, "arm", 0 );
  link.setBody( &body );
  link.setMaterial( &material );
  QString text;
  QTextStream stream( &text );

  link.writeToFileTransfer( stream );
  stream.flush();

  QCOMPARE( text, QString( "Link arm 0 0 0 0 0 0 1 0 0 0 1 0 0 0 1 n n \"left\tarm.wrl\" steel n\n" ) );
}

void SIGEL_Robot::TST_SIG_Link::unstreamingConstructorReadsABodyNameWithWhitespaceAndStopsAfterTheLink()
{
  SIG_Robot robot;
  SIG_Body *body = new SIG_Body( &robot, "left\tarm.wrl", "parts/" );
  SIG_Material *material = new SIG_Material( &robot, "steel" );

  // SIG_Robot owns the parts and frees them.
  robot.addBody( body );
  robot.addMaterial( material );
  SIG_Link written( &robot, "arm", 0 );
  written.setBody( body );
  written.setMaterial( material );
  QString text;
  QTextStream out( &text );
  written.writeToFileTransfer( out );
  out << "Joint";
  out.flush();
  QTextStream in( &text );
  QString keyword;
  in >> keyword;
  QString thrown;

  std::unique_ptr< SIG_Link > read = unstreamedLink( robot, in, thrown );
  QString next;
  in >> next;

  QCOMPARE( thrown, QString() );
  QCOMPARE( read->getName(), QString( "arm" ) );
  QCOMPARE( read->getBody(), body );
  QCOMPARE( read->getMaterial(), material );
  QCOMPARE( next, QString( "Joint" ) );
}
