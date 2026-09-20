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
#include "SIGEL_RobotIO/SIG_RobotScanner.h"
#include "SIGEL_RobotIO/SIG_RobotIOExceptions.h"
#ifdef _WINDOWS
#include <iostream.h>
#else
#include <iostream>
#endif

namespace SIGEL_RobotIO {
        namespace RobotSymbol {
        int const openingBrace = 1;
        int const closingBrace = 2;
        int const openingParen = 3;
        int const closingParen = 4;
        int const word = 5;
        int const number = 6;
        int const string = 7;
        int const semicolon = 8;
        int const comma = 9;
        int const equals = 10;
        int const slash = 11;
        }
        
        SIG_RobotScanner::SIG_RobotScanner (QString liesdas)
                : SIG_Scanner (liesdas)
        { }

        SIG_RobotScanner::~SIG_RobotScanner (void)
        { }

        void SIG_RobotScanner::nextSymbol (void)
        { 
                skipWhiteSpace ();
                if (position >= text.length ()) {
                        currentSymbol = "(EOS)";
                        currentSymType = Symbol::EOS;
                } else if (text.at (position) == '{') {
                        position++;
                        currentSymbol = "{";
                        currentSymType = RobotSymbol::openingBrace;
                } else if (text.at (position) == '}') {
                        position++;
                        currentSymbol = "}";
                        currentSymType = RobotSymbol::closingBrace;
                } else if (text.at (position) == '(') {
                        position++;
                        currentSymbol = "(";
                        currentSymType = RobotSymbol::openingParen;
                } else if (text.at (position) == ')') {
                        position++;
                        currentSymbol = ")";
                        currentSymType = RobotSymbol::closingParen;
                } else if (text.at (position).isLetter ()) {
                        int fp = position;
                        position++;
                        while ((position < text.length ()) &&
                               (text.at (position).isLetterOrNumber () ||
                                (text.at (position) == '_')))
                                position++;
                        currentSymbol = text.mid (fp, position - fp);
                        currentSymType = RobotSymbol::word;
                } else if ((text.at (position) == '-') ||
                           (text.at (position) == '+') ||
                           (text.at (position).isNumber ())) {
                        int fp = position;
                        position++;
                        while ((position < text.length ()) &&
                               text.at (position).isNumber ())
                                position++;
                        if ((position < text.length ()) &&
                            (text.at (position) == '.')) {
                                position++;
                                while ((position < text.length ()) &&
                                       text.at (position).isNumber ())
                                        position++;
                        }
                        if ((position < text.length ()) &&
                            ((text.at (position) == 'e') ||
                             (text.at (position) == 'E'))) {
                                position++;
                                if ((position < text.length ()) &&
                                    ((text.at (position) == '+') ||
                                     (text.at (position) == '-')))
                                        position++;
                                while ((position < text.length ()) &&
                                       text.at (position).isNumber ())
                                        position++;
                        }
                        currentSymbol = text.mid (fp, position - fp);
                        currentSymType = RobotSymbol::number;
                } else if (text.at (position) == '\"') {
                        int fp = position;
                        position++;
                        while ((position < text.length ()) &&
                               (text.at (position) != '\"')) {
                                if ((text.at (position) == '\n') ||
                                    (text.at (position) == '\r'))
                                        throw SIG_SyntaxError (__FILE__, __LINE__,
                                                               "String contains EOL",
                                                               "(unknown)", lineposition);
                                position++;
                        }
                        if (position >= text.length ())
                                throw SIG_SyntaxError (__FILE__, __LINE__,
                                                       "Unexpected end of text in string scanning",
                                                       "(unknown)", lineposition);
                        position++;
                        currentSymbol = text.mid (fp + 1, position - fp - 2);
                        currentSymType = RobotSymbol::string;
                } else if (text.at (position) == ';') {
                        position++;
                        currentSymbol = ";";
                        currentSymType = RobotSymbol::semicolon;
                } else if (text.at (position) == ',') {
                        position++;
                        currentSymbol = ",";
                        currentSymType = RobotSymbol::comma;
                } else if (text.at (position) == '=') {
                        position++;
                        currentSymbol = "=";
                        currentSymType = RobotSymbol::equals;
                } else if (text.at (position) == '/') {
                        position++;
                        currentSymbol = "/";
                        currentSymType = RobotSymbol::slash;
                } else {
                        throw SIG_SyntaxError (__FILE__, __LINE__,
                                               "Unknown symbol.",
                                               "(unknown)", lineposition);
                }
        }
}
