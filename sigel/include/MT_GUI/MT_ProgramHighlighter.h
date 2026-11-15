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
#ifndef MT_GUI_MT_PROGRAMHIGHLIGHTER_H
#define MT_GUI_MT_PROGRAMHIGHLIGHTER_H

#include "SIGEL_CommonGUI/SIG_ProgramHighlighter.h"

class MT_ProgramHighlighter : public SIGEL_CommonGUI::SIG_ProgramHighlighter
{
public:
	MT_ProgramHighlighter(QTextDocument *document) : SIG_ProgramHighlighter(document) {}

protected:
	QStringList controlFlowOpcodes() const override { return { "cmp", "jmp", "jle", "jeq" }; }
	QStringList arithmeticOpcodes() const override { return { "add", "sub", "mul", "div", "mod", "min", "max" }; }
	QStringList dataOpcodes() const override { return { "cpy" }; }
	QStringList robotOpcodes() const override { return { "swt", "lui", "lus", "lud", "lum" }; }
	QStringList nopOpcodes() const override { return { "nop" }; }
};

#endif // MT_GUI_MT_PROGRAMHIGHLIGHTER_H
