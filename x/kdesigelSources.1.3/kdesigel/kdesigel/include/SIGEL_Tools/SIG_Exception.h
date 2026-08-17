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
#ifndef SIGEL_TOOLS_SIG_EXCEPTION_H
#define SIGEL_TOOLS_SIG_EXCEPTION_H

#include <qstring.h>

namespace SIGEL_Tools
{

  /**
   * This is the abstract superclass of all exception-classes
   * thrown by SIGEL-classes.
   */
  class SIG_Exception
    {
    public:
      /**
       * This constructor should be used when throwing a new Exception.
       *
       * @param filename The filename where the exception occured.
       *                 The macro __FILE__ should be inserted here.
       * @param line     The line of code where the exception occured.
       *                 The macro __LINE__ should be inserted here.
       * @param message  A short description of the exception.
       */
      SIG_Exception(QString fileName,
		    int line,
		    QString message);

      /**
       * This constructor should be used when throwing an exception
       * in a catch-block. The resulting exception contains the
       * message from prevException, the sort-name of exception
       * and the newly supplied message.
       * It enables to add a (more detailed) message
       * to the ones already contained in the catched exception. The
       * created exception containing the accumulated messages can
       * be thrown again.
       *
       * @param fileName The filename where the exception occured.
       *                 The macro __FILE__ should be inserted here.
       * @param line     The line of code where the exception occured.
       *                 The macro __LINE__ should be inserted here.
       * @param message  A short description of the exception.
       * @prevException  The catched exception whose message
       *                 (together with the newly supplied one)
       *                 forms the message of the constructed exception.
       */
      SIG_Exception(QString fileName,
		    int line,
		    QString message,
		    SIG_Exception const& prevException);

      /**
       * @return The message contained in this exception.
       */
      QString getMessage() const;

    protected:
      /**
       * The QString containing all textual information
       * about this exception.
       */
      QString message;
    };

}

#endif // SIGEL_TOOLS_SIG_EXCEPTION_H
