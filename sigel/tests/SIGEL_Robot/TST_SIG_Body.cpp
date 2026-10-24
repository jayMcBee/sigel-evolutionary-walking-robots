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
#include "SIGEL_Robot/TST_SIG_Body.h"

#include "SIGEL_Robot/SIG_Body.h"
#include "SIGEL_Robot/SIG_Geometry.h"
#include "SIGEL_Robot/SIG_RobotExceptions.h"
#include "SIGEL_Robot/SIG_Robot.h"

#include <QFile>
#include <QTemporaryDir>
#include <QtTest>

namespace {

  const char *const triangle =
    "Shape { geometry IndexedFaceSet { coord Coordinate { point [ 0 0 0, 1 0 0, 0 1 0 ] }\n"
    "                                  coordIndex [ 0, 1, 2, -1 ] } }\n";

  QString writeVRMLFile( const QTemporaryDir &folder, const QString &body )
  {
    QString fileName = folder.filePath( "body.wrl" );
    QFile file( fileName );
    // A file that is not written fails the test: load finds no face set.
    if ( !file.open( QIODevice::WriteOnly ) )
      return fileName;
    file.write( ( "#VRML V2.0 utf8\n" + body ).toUtf8() );
    return fileName;
  }

}

void SIGEL_Robot::TST_SIG_Body::loadReadsTheFaceSetsOfAFileWithUse()
{
  QTemporaryDir folder;
  // USE inside a children list, and after a "}" that a comma ends.
  QString fileName = writeVRMLFile( folder,
                                    QString( "DEF T Transform { children [ " ) + triangle + " ] }\n"
                                    "Transform { children [ USE T ] }\n"
                                    "Transform { } ,\n"
                                    "USE T\n" );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  body.load();

  QVERIFY( body.getGeometry() );
  QCOMPARE( body.getGeometry()->getNumPolygons(), 3 );
}

void SIGEL_Robot::TST_SIG_Body::loadRefusesAUseDirectlyAfterANodeEnd()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder,
                                    QString( "DEF S " ) + triangle +
                                    "USE S\n" );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  try {
    body.load();
    QFAIL( "load did not throw" );
  }
  catch ( const SIG_GeometryFileError &e ) {
    QVERIFY2( e.getMessage().contains( "line 4:" ), qPrintable( e.getMessage() ) );
    QVERIFY2( e.getMessage().contains( fileName ), qPrintable( e.getMessage() ) );
  }
  QCOMPARE( body.getGeometry(), nullptr );
}

void SIGEL_Robot::TST_SIG_Body::loadIgnoresUseInCommentsAndStrings()
{
  QTemporaryDir folder;
  QString fileName = writeVRMLFile( folder,
                                    QString( triangle ) +
                                    "# } USE S\n"
                                    "WorldInfo { title \"x\" } WorldInfo { title \"} USE S\" }\n" );
  SIG_Robot robot;
  SIG_Body body( &robot, fileName, "" );

  body.load();

  QCOMPARE( body.getGeometry()->getNumPolygons(), 1 );
}
