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

	SIG_RegisterOverlay::SIG_RegisterOverlay( SIGEL_Simulation::SIG_Interpreter const &interpreter )
		: interpreter( interpreter ),
		  minimumValue( interpreter.getRegisters().first().getMinValue() ),
		  maximumValue( interpreter.getRegisters().first().getMaxValue() ),
		  font( QFontDatabase::systemFont( QFontDatabase::FixedFont ) )
	{
		font.setPointSize( 8 );
		QFontMetrics const metrics( font );

		QString const widestLabel = QString( "R%1" ).arg( interpreter.getRegisters().size() - 1 );
		int const valueCharacters = qMax( QString::number( minimumValue ).size(), QString::number( maximumValue ).size() );
		labelWidth = metrics.horizontalAdvance( widestLabel );
		valueWidth = metrics.horizontalAdvance( QString( valueCharacters, '0' ) );
		columnWidth = margin + labelWidth + columnGap + trackWidth + columnGap + valueWidth + margin;

		labelX = margin;
		trackX = labelX + labelWidth + columnGap;
		trackMiddle = trackX + trackWidth / 2;
		valueX = trackX + trackWidth + columnGap;
	}

	QColor SIG_RegisterOverlay::white( int opacityPercent )
	{
		return QColor( 255, 255, 255, 255 * opacityPercent / 100 );
	}

	QImage SIG_RegisterOverlay::render( int height, qreal devicePixelRatio ) const
	{
		QImage image( qRound( columnWidth * devicePixelRatio ), qRound( height * devicePixelRatio ), QImage::Format_RGBA8888 );
		image.setDevicePixelRatio( devicePixelRatio );
		image.fill( QColor( 0x2b, 0x2b, 0x2b ) );

		QPainter painter( &image );
		painter.setFont( font );

		int y = margin;
		drawTextRow( painter, y, "PC", QString::number( interpreter.getProgramCounter() ) );
		y += rowHeight;
		drawTextRow( painter, y, "CF", interpreter.getCompareFlag() ? "1" : "0" );
		y += rowHeight;

		QList< SIGEL_Simulation::SIG_Register > const &registers = interpreter.getRegisters();

		for ( int index = 0; index < registers.size(); index++ )
		{
			bool const startsGroup = index % registersPerGroup == 0;

			// The rows that do not fit in a short view are left out.
			if ( y + ( startsGroup ? groupGap : 0 ) + rowHeight > height )
				break;

			if ( startsGroup )
			{
				drawGroupLine( painter, y );
				y += groupGap;
			}

			drawRegisterRow( painter, y, index, registers[index].getValue() );
			y += rowHeight;
		}

		return image;
	}

	void SIG_RegisterOverlay::drawTextRow( QPainter &painter, int y, QString const &label, QString const &value ) const
	{
		painter.setPen( white( 38 ) );
		painter.drawText( QRect( labelX, y, labelWidth, rowHeight ), Qt::AlignLeft | Qt::AlignVCenter, label );

		// The program counter and compare flag have no bar; their values may use the track's width.
		painter.setPen( white( 80 ) );
		painter.drawText( QRect( trackX, y, valueX + valueWidth - trackX, rowHeight ), Qt::AlignRight | Qt::AlignVCenter, value );
	}

	void SIG_RegisterOverlay::drawGroupLine( QPainter &painter, int y ) const
	{
		painter.setPen( white( 10 ) );
		painter.drawLine( margin, y + groupGap / 2, columnWidth - margin, y + groupGap / 2 );
	}

	void SIG_RegisterOverlay::drawRegisterRow( QPainter &painter, int y, int index, int value ) const
	{
		int const middle = y + rowHeight / 2;

		painter.setPen( white( 38 ) );
		painter.drawText( QRect( labelX, y, labelWidth, rowHeight ), Qt::AlignLeft | Qt::AlignVCenter,
		                  QString( "R%1" ).arg( index ) );

		int const trackTop = middle - trackHeight / 2;
		painter.fillRect( trackX, trackTop, trackWidth, trackHeight, white( 6 ) );

		int const bar = barWidth( value );
		painter.fillRect( qMin( trackMiddle, trackMiddle + bar ), trackTop, qAbs( bar ), trackHeight, white( 50 ) );

		painter.setPen( white( 15 ) );
		painter.drawLine( trackMiddle, trackTop, trackMiddle, trackTop + trackHeight - 1 );

		painter.setPen( white( 80 ) );
		painter.drawText( QRect( valueX, y, valueWidth, rowHeight ), Qt::AlignRight | Qt::AlignVCenter,
		                  QString::number( value ) );
	}

	int SIG_RegisterOverlay::barWidth( int value ) const
	{
		if ( value > 0 )
			return static_cast< int >( std::lround( value * ( trackWidth / 2.0 ) / maximumValue ) );

		if ( value < 0 )
			return -static_cast< int >( std::lround( value * ( trackWidth / 2.0 ) / minimumValue ) );

		return 0;
	}

}
