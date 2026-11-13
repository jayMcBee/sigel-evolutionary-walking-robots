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
#ifndef SIGEL_SLAVEGUI_SIG_REGISTEROVERLAY_H
#define SIGEL_SLAVEGUI_SIG_REGISTEROVERLAY_H

#include "SIGEL_Simulation/SIG_Interpreter.h"

#include <QColor>
#include <QFont>
#include <QImage>
#include <QString>

class QPainter;

namespace SIGEL_SlaveGUI
{

	/**
	 * Draws the program counter, the compare flag and the registers of the robot's
	 * interpreter as a column at the right edge of the view.
	 */
	class SIG_RegisterOverlay
	{
	public:

		/**
		 * Measures the column for the registers of the given interpreter.
		 */
		SIG_RegisterOverlay( SIGEL_Simulation::SIG_Interpreter const &interpreter );

		/**
		 * Draws the column for a view of the given height. The image has the device pixel ratio of the view.
		 */
		QImage render( int height, qreal devicePixelRatio ) const;

	private:

		static constexpr int registersPerGroup = 8;
		static constexpr int groupGap = 6;
		static constexpr int rowHeight = 21;
		static constexpr int margin = 8;
		static constexpr int columnGap = 12;
		static constexpr int trackWidth = 32;
		static constexpr int trackHeight = 9;

		static QColor white( int opacityPercent );

		void drawTextRow( QPainter &painter, int y, QString const &label, QString const &value ) const;

		void drawGroupLine( QPainter &painter, int y ) const;

		void drawRegisterRow( QPainter &painter, int y, int index, int value ) const;

		/**
		 * The bar grows from the middle: right for positive values, left for negative ones.
		 */
		int barWidth( int value ) const;

		SIGEL_Simulation::SIG_Interpreter const &interpreter;
		int minimumValue;
		int maximumValue;
		QFont font;
		int labelWidth;
		int valueWidth;
		int columnWidth;
		int labelX;
		int trackX;
		int trackMiddle;
		int valueX;
	};

}

#endif // SIGEL_SLAVEGUI_SIG_REGISTEROVERLAY_H
