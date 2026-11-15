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
#ifndef SIGEL_COMMONGUI_SIG_PROGRAMVIEW_H
#define SIGEL_COMMONGUI_SIG_PROGRAMVIEW_H

#include <QPlainTextEdit>

namespace SIGEL_CommonGUI
{

  class SIG_ProgramHighlighter;

  class SIG_ProgramView : public QPlainTextEdit
    {
    public:
      SIG_ProgramView( QWidget *parent );

      void setHighlighter( SIG_ProgramHighlighter *programHighlighter );

    private:
      void changeEvent( QEvent *event ) override;
      bool eventFilter( QObject *watched, QEvent *event ) override;
      void resizeEvent( QResizeEvent *event ) override;

      int lastLineNumber() const;
      int lineNumberAreaWidth() const;
      void paintLineNumbers( QPaintEvent *event );
      void updateLineNumberAreaWidth();
      void updateLineNumberArea( const QRect &rect, int dy );
      void setSchemeFromPalette();

      QWidget *lineNumberArea;
      SIG_ProgramHighlighter *highlighter;
    };

}

#endif // SIGEL_COMMONGUI_SIG_PROGRAMVIEW_H
