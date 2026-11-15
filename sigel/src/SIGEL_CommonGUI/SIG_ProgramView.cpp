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
#include "SIGEL_CommonGUI/SIG_ProgramView.h"

#include "SIGEL_CommonGUI/SIG_ProgramHighlighter.h"

#include <QFontDatabase>
#include <QPainter>
#include <QTextBlock>

namespace SIGEL_CommonGUI
{

  SIG_ProgramView::SIG_ProgramView( QWidget *parent )
    : QPlainTextEdit( parent ),
      lineNumberArea( new QWidget( this ) ),
      highlighter( new SIG_ProgramHighlighter( document() ) )
  {
    setReadOnly( true );
    setLineWrapMode( QPlainTextEdit::NoWrap );

    QFont fixedFont = QFontDatabase::systemFont( QFontDatabase::FixedFont );
    fixedFont.setPointSize( 9 );
    setFont( fixedFont );

    lineNumberArea->installEventFilter( this );
    connect( this, &QPlainTextEdit::blockCountChanged, this, &SIG_ProgramView::updateLineNumberAreaWidth );
    connect( this, &QPlainTextEdit::updateRequest, this, &SIG_ProgramView::updateLineNumberArea );
    updateLineNumberAreaWidth();

    setSchemeFromPalette();
  }

  void SIG_ProgramView::setHighlighter( SIG_ProgramHighlighter *programHighlighter )
  {
    delete highlighter;
    highlighter = programHighlighter;
    setSchemeFromPalette();
  }

  void SIG_ProgramView::changeEvent( QEvent *event )
  {
    QPlainTextEdit::changeEvent( event );
    if( event->type() == QEvent::PaletteChange )
      setSchemeFromPalette();
  }

  bool SIG_ProgramView::eventFilter( QObject *watched, QEvent *event )
  {
    if( watched != lineNumberArea || event->type() != QEvent::Paint )
      return QPlainTextEdit::eventFilter( watched, event );

    paintLineNumbers( static_cast<QPaintEvent *>( event ) );
    return true;
  }

  void SIG_ProgramView::resizeEvent( QResizeEvent *event )
  {
    QPlainTextEdit::resizeEvent( event );
    updateLineNumberAreaWidth();
  }

  int SIG_ProgramView::lastLineNumber() const
  {
    // Every program line ends with a line break, which leaves an empty block at the end. An empty view gives -1.
    int last = blockCount() - 1;
    if( document()->lastBlock().text().isEmpty() )
      --last;
    return last;
  }

  int SIG_ProgramView::lineNumberAreaWidth() const
  {
    int digits = QString::number( qMax( 0, lastLineNumber() ) ).length();
    return fontMetrics().horizontalAdvance( QLatin1Char( '9' ) ) * ( digits + 1 );
  }

  void SIG_ProgramView::paintLineNumbers( QPaintEvent *event )
  {
    QPainter painter( lineNumberArea );
    painter.fillRect( event->rect(), palette().window() );
    painter.setPen( palette().color( QPalette::PlaceholderText ) );

    int numberWidth = lineNumberArea->width() - fontMetrics().horizontalAdvance( QLatin1Char( '9' ) ) / 2;
    int last = lastLineNumber();
    QTextBlock block = firstVisibleBlock();
    int top = qRound( blockBoundingGeometry( block ).translated( contentOffset() ).top() );
    while( block.isValid() && block.blockNumber() <= last && top <= event->rect().bottom() )
    {
      painter.drawText( 0, top, numberWidth, fontMetrics().height(), Qt::AlignRight, QString::number( block.blockNumber() ) );
      top += qRound( blockBoundingRect( block ).height() );
      block = block.next();
    }
  }

  void SIG_ProgramView::updateLineNumberAreaWidth()
  {
    int width = lineNumberAreaWidth();
    setViewportMargins( width, 0, 0, 0 );

    QRect contents = contentsRect();
    lineNumberArea->setGeometry( contents.left(), contents.top(), width, contents.height() );
  }

  void SIG_ProgramView::updateLineNumberArea( const QRect &rect, int dy )
  {
    if( dy != 0 )
    {
      lineNumberArea->scroll( 0, dy );
      return;
    }
    lineNumberArea->update( 0, rect.y(), lineNumberArea->width(), rect.height() );
  }

  void SIG_ProgramView::setSchemeFromPalette()
  {
    highlighter->setDarkScheme( palette().color( QPalette::Base ).lightness() < 128 );
  }

}
