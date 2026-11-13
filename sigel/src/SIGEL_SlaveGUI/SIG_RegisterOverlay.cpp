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
#include "SIGEL_SlaveGUI/SIG_RegisterOverlay.h"

#include <QFont>
#include <QFontDatabase>
#include <QFontMetrics>
#include <QPainter>
#include <QString>

#include <cmath>

namespace SIGEL_SlaveGUI
{

	QColor SIG_RegisterOverlay::white( int opacityPercent )
	{
		return QColor( 255, 255, 255, 255 * opacityPercent / 100 );
	}

	QImage SIG_RegisterOverlay::render( SIGEL_Simulation::SIG_Interpreter const &interpreter, int height, qreal devicePixelRatio )
	{
		QList< SIGEL_Simulation::SIG_Register > const &registers = interpreter.getRegisters();

		int const registersPerGroup = 8;
		int const groupGap = 6;
		int const rowHeight = 21;
		int const margin = 8;
		int const columnGap = 12;
		int const trackWidth = 32;
		int const trackHeight = 9;

		int const minimumValue = registers.first().getMinValue();
		int const maximumValue = registers.first().getMaxValue();

		QFont font = QFontDatabase::systemFont( QFontDatabase::FixedFont );
		font.setPointSize( 8 );
		QFontMetrics const metrics( font );

		QString const widestLabel = QString( "R%1" ).arg( registers.size() - 1 );
		int const valueCharacters = qMax( QString::number( minimumValue ).size(), QString::number( maximumValue ).size() );
		int const labelWidth = metrics.horizontalAdvance( widestLabel );
		int const valueWidth = metrics.horizontalAdvance( QString( valueCharacters, '0' ) );
		int const columnWidth = margin + labelWidth + columnGap + trackWidth + columnGap + valueWidth + margin;

		QImage image( qRound( columnWidth * devicePixelRatio ), qRound( height * devicePixelRatio ), QImage::Format_RGBA8888 );
		image.setDevicePixelRatio( devicePixelRatio );
		image.fill( QColor( 0x2b, 0x2b, 0x2b ) );

		QPainter painter( &image );
		painter.setFont( font );

		int const labelX = margin;
		int const trackX = labelX + labelWidth + columnGap;
		int const trackMiddle = trackX + trackWidth / 2;
		int const valueX = trackX + trackWidth + columnGap;

		// The program counter and compare flag have no bar; their values may use the track's width.
		auto drawTextRow = [&]( QString const &label, QString const &value, int rowTop )
		{
			painter.setPen( white( 38 ) );
			painter.drawText( QRect( labelX, rowTop, labelWidth, rowHeight ), Qt::AlignLeft | Qt::AlignVCenter, label );
			painter.setPen( white( 80 ) );
			painter.drawText( QRect( trackX, rowTop, valueX + valueWidth - trackX, rowHeight ), Qt::AlignRight | Qt::AlignVCenter, value );
		};

		int y = margin;
		drawTextRow( "PC", QString::number( interpreter.getProgramCounter() ), y );
		y += rowHeight;
		drawTextRow( "CF", interpreter.getCompareFlag() ? "1" : "0", y );
		y += rowHeight;

		for ( int index = 0; index < registers.size(); index++ )
		{
			bool const startsGroup = index % registersPerGroup == 0;

			// The rows that do not fit in a short view are left out.
			if ( y + ( startsGroup ? groupGap : 0 ) + rowHeight > height )
				break;

			if ( startsGroup )
			{
				painter.setPen( white( 10 ) );
				painter.drawLine( margin, y + groupGap / 2, columnWidth - margin, y + groupGap / 2 );
				y += groupGap;
			}

			int const value = registers[index].getValue();
			int const middle = y + rowHeight / 2;

			painter.setPen( white( 38 ) );
			painter.drawText( QRect( labelX, y, labelWidth, rowHeight ), Qt::AlignLeft | Qt::AlignVCenter,
			                  QString( "R%1" ).arg( index ) );

			int const trackTop = middle - trackHeight / 2;
			painter.fillRect( trackX, trackTop, trackWidth, trackHeight, white( 6 ) );

			// The bar grows from the middle: right for positive values, left for negative ones.
			int barWidth = 0;
			if ( value > 0 )
				barWidth = static_cast< int >( std::lround( value * ( trackWidth / 2.0 ) / maximumValue ) );
			if ( value < 0 )
				barWidth = -static_cast< int >( std::lround( value * ( trackWidth / 2.0 ) / minimumValue ) );

			painter.fillRect( qMin( trackMiddle, trackMiddle + barWidth ), trackTop, qAbs( barWidth ), trackHeight, white( 50 ) );

			painter.setPen( white( 15 ) );
			painter.drawLine( trackMiddle, trackTop, trackMiddle, trackTop + trackHeight - 1 );

			painter.setPen( white( 80 ) );
			painter.drawText( QRect( valueX, y, valueWidth, rowHeight ), Qt::AlignRight | Qt::AlignVCenter,
			                  QString::number( value ) );

			y += rowHeight;
		}

		return image;
	}

}
