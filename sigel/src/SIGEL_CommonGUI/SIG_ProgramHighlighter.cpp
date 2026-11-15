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
#include "SIGEL_CommonGUI/SIG_ProgramHighlighter.h"

#include <QRegularExpression>

namespace SIGEL_CommonGUI
{

  SIG_ProgramHighlighter::SIG_ProgramHighlighter( QTextDocument *document )
    : QSyntaxHighlighter( document )
  {
  }

  void SIG_ProgramHighlighter::setDarkScheme( bool dark )
  {
    darkScheme = dark;
    rehighlight();
  }

  QColor SIG_ProgramHighlighter::opcodeColour( const QString &opcode ) const
  {
    // Colours from KDE's Breeze Dark and Breeze Light: ControlFlow, Keyword, DataType, Function, Comment.
    if( controlFlowOpcodes().contains( opcode ) )
      return darkScheme ? QColor( "#fdbc4b" ) : QColor( "#1f1c1b" );

    if( arithmeticOpcodes().contains( opcode ) )
      return darkScheme ? QColor( "#cfcfc2" ) : QColor( "#1f1c1b" );

    if( dataOpcodes().contains( opcode ) )
      return darkScheme ? QColor( "#2980b9" ) : QColor( "#0057ae" );

    if( robotOpcodes().contains( opcode ) )
      return darkScheme ? QColor( "#8e44ad" ) : QColor( "#644a9b" );

    if( nopOpcodes().contains( opcode ) )
      return darkScheme ? QColor( "#7a7c7d" ) : QColor( "#898887" );

    return QColor();
  }

  QColor SIG_ProgramHighlighter::numberColour() const
  {
    // Breeze DecVal.
    return darkScheme ? QColor( "#f67400" ) : QColor( "#b08000" );
  }

  void SIG_ProgramHighlighter::highlightBlock( const QString &text )
  {
    int opcodeLength = text.indexOf( ' ' );
    if( opcodeLength < 0 )
      opcodeLength = text.length();

    QColor colour = opcodeColour( text.left( opcodeLength ) );
    if( colour.isValid() )
      setFormat( 0, opcodeLength, colour );

    static const QRegularExpression number( "-?\\d+(\\.\\d+)?([eE][-+]?\\d+)?" );
    QRegularExpressionMatchIterator matches = number.globalMatch( text, opcodeLength );
    while( matches.hasNext() )
    {
      QRegularExpressionMatch match = matches.next();
      setFormat( match.capturedStart(), match.capturedLength(), numberColour() );
    }
  }

}
