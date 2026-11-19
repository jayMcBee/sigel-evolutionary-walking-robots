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
#ifndef SIGEL_TOOLS_TST_SIG_LOGGING_H
#define SIGEL_TOOLS_TST_SIG_LOGGING_H

#include <QObject>
#include <QString>
#include <QtGlobal>

#include <vector>

namespace SIGEL_Tools
{

  /**
   * One message that reached the message handler of TST_SIG_Logging.
   */
  struct TST_SIG_LoggedMessage
  {
    QtMsgType type;
    QString category;
    QString file;
    int line;
    QString message;

    /** The message as the message pattern writes it. */
    QString formattedMessage;
  };

  /**
   * The unit tests of SIG_Logging.
   *
   * Each test installs a message handler that keeps the messages, and puts
   * the message handler, the message pattern and the filter rules back after.
   */
  class TST_SIG_Logging : public QObject
  {
    Q_OBJECT

  private slots:

    void init();

    void cleanup();

    void debugIsOffAfterInitializeLogging();

    void infoWarningAndCriticalAreOnAfterInitializeLogging();

    void disabledDebugDoesNotEvaluateItsArguments();

    void messageHasTheGivenCategory();

    void messageIsWrittenWithoutQuotesOrSpaces();

    void messageKeepsTheFileAndLineOfTheCall();

    void everyLineIsDateLevelCategoryAndMessage();

    void ruleForACategoryReachesEveryCallSiteOfThatNameOnly();

  private:

    static void keepMessage( QtMsgType type, const QMessageLogContext &context, const QString &message );

    static std::vector<TST_SIG_LoggedMessage> loggedMessages;
    QtMessageHandler previousHandler = nullptr;
  };

}

#endif // SIGEL_TOOLS_TST_SIG_LOGGING_H
