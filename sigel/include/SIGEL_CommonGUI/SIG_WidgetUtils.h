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
#ifndef SIGEL_COMMONGUI_SIG_WIDGETUTILS_H
#define SIGEL_COMMONGUI_SIG_WIDGETUTILS_H

#include <QObject>

namespace SIGEL_CommonGUI
{

	/**
	 * Helpers for working with Qt widgets.
	 */
	class SIG_WidgetUtils
	{
	public:

		/**
		 * Every validator under owner takes the format toInt() and toDouble() read:
		 * a dot for decimals, no thousands separator.
		 */
		static void useDotDecimalNumberFormat( QObject *owner );
	};

}

#endif // SIGEL_COMMONGUI_SIG_WIDGETUTILS_H
