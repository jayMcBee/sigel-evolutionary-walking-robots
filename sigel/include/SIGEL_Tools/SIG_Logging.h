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
#ifndef SIGEL_TOOLS_SIG_LOGGING_H
#define SIGEL_TOOLS_SIG_LOGGING_H

#include <QLoggingCategory>

// Macros, not methods: a disabled level does not evaluate the arguments, and Qt records file and line.
// The lambda gives each call site its own static category of that name; Qt applies the rules to all of them.
// "" name compiles only for a string literal, the one name that lives as long as the category.
#define SIG_LOG_CATEGORY( name ) ( []() -> const QLoggingCategory & { static const QLoggingCategory category( "" name ); return category; } )

#define SIG_LOG_DEBUG( name ) qCDebug( SIG_LOG_CATEGORY( name ) ).noquote().nospace()
#define SIG_LOG_INFO( name ) qCInfo( SIG_LOG_CATEGORY( name ) ).noquote().nospace()
#define SIG_LOG_WARNING( name ) qCWarning( SIG_LOG_CATEGORY( name ) ).noquote().nospace()
#define SIG_LOG_CRITICAL( name ) qCCritical( SIG_LOG_CATEGORY( name ) ).noquote().nospace()

namespace SIGEL_Tools
{

	/**
	 * SIGEL's logging, built on Qt's. The caller names the category after its
	 * namespace, as a string literal.
	 *
	 * Usage: SIG_LOG_WARNING( "SIGEL_GP" ) << "terrain file not found: " << fileName;
	 * Debug for one namespace: QT_LOGGING_RULES="SIGEL_GP.debug=true"
	 */
	class SIG_Logging
	{
	public:

		/** Sets the message pattern (type first, file and line on debug lines only) and switches debug off; call it first in main. */
		static void initializeLogging();
	};

}

#endif // SIGEL_TOOLS_SIG_LOGGING_H
