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
#include "SIGEL_GP/SIG_GPPVMHost.h"

#include <qtextstream.h>

namespace SIGEL_GP
{

  SIG_GPPVMHost::SIG_GPPVMHost()
    : name(),
      maxSlaves( 1 ),
      enabled( false ),
      executableDir()
  { };

  SIG_GPPVMHost::SIG_GPPVMHost( QString name,
				int maxSlaves,
				bool enabled,
				QDir executableDir )
    : name( name ),
      maxSlaves( maxSlaves ),
      enabled( enabled ),
      executableDir( executableDir )
  { };

  // Qt 2's QTextStream::operator>>(char&) was `c = eat_ws()' -- skip whitespace,
  // then return the first non-whitespace character (qtextstream.cpp:212,1029).
  // Qt 6's returns the very next character, whitespace included. Reading a
  // PVMHOST line with Qt 6's semantics makes the two priming reads consume the
  // space and then the opening quote, so the loop below sees '"' immediately and
  // EVERY host parsed from an .exp file gets an empty slave directory.
  //
  // Note this reproduces the Qt 2 behaviour exactly, INCLUDING inside the quoted
  // path: each read skips whitespace, so 1.3 silently drops spaces in a
  // directory name (/tmp/with space -> /tmp/withspace). That is a defect and it
  // is preserved, not fixed.
  static void readCharQt2( QTextStream &stream, char &c )
  {
    stream.skipWhiteSpace();
    stream >> c;
  }

  SIG_GPPVMHost::SIG_GPPVMHost( QString input )
  {
    QTextStream inputStream( &input, QIODeviceBase::ReadOnly );

    int enabledInt = 0;

    char buffer = 0;

    inputStream >> name
		>> maxSlaves
		>> enabledInt;
    readCharQt2( inputStream, buffer );
    readCharQt2( inputStream, buffer );

    QString dirString;

    while (buffer != '\"')
      {
	dirString.append( buffer );
	readCharQt2( inputStream, buffer );
      };

    enabled = enabledInt;
    executableDir.setPath( dirString );
  };

  QString SIG_GPPVMHost::print() const
  {
    QString result;

    QTextStream resultStream( &result, QIODeviceBase::WriteOnly );

    resultStream << name
		 << " "
		 << maxSlaves
		 << " "
		 << enabled
		 << " "
		 << "\""
		 << executableDir.path()
		 << "\""
		 << "\n";

    return result;
  };

}
