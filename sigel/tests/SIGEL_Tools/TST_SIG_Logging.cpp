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
#include "SIGEL_Tools/TST_SIG_Logging.h"

#include "SIGEL_Tools/SIG_Logging.h"

#include <QLoggingCategory>
#include <QRegularExpression>
#include <QtTest>

std::vector<SIGEL_Tools::TST_SIG_LoggedMessage> SIGEL_Tools::TST_SIG_Logging::loggedMessages;

void SIGEL_Tools::TST_SIG_Logging::init()
{
  loggedMessages.clear();
  previousHandler = qInstallMessageHandler( keepMessage );
}

void SIGEL_Tools::TST_SIG_Logging::cleanup()
{
  qInstallMessageHandler( previousHandler );

  // Qt's default pattern; an empty pattern would write empty lines.
  qSetMessagePattern( "%{if-category}%{category}: %{endif}%{message}" );
  QLoggingCategory::setFilterRules( QString() );
}

void SIGEL_Tools::TST_SIG_Logging::keepMessage( QtMsgType type, const QMessageLogContext &context, const QString &message )
{
  loggedMessages.push_back( { type, QString::fromLatin1( context.category ), QString::fromLatin1( context.file ), context.line, message, qFormatLogMessage( type, context, message ) } );
}

void SIGEL_Tools::TST_SIG_Logging::debugIsOffAfterInitializeLogging()
{
  SIG_Logging::initializeLogging();

  SIG_LOG_DEBUG( "SIGEL_Tools" ) << "debug";

  QVERIFY( loggedMessages.empty() );
}

void SIGEL_Tools::TST_SIG_Logging::infoWarningAndCriticalAreOnAfterInitializeLogging()
{
  SIG_Logging::initializeLogging();

  SIG_LOG_INFO( "SIGEL_Tools" ) << "info";
  SIG_LOG_WARNING( "SIGEL_Tools" ) << "warning";
  SIG_LOG_CRITICAL( "SIGEL_Tools" ) << "critical";

  QCOMPARE( loggedMessages.size(), size_t( 3 ) );
  QCOMPARE( loggedMessages[0].type, QtInfoMsg );
  QCOMPARE( loggedMessages[0].message, QString( "info" ) );
  QCOMPARE( loggedMessages[1].type, QtWarningMsg );
  QCOMPARE( loggedMessages[1].message, QString( "warning" ) );
  QCOMPARE( loggedMessages[2].type, QtCriticalMsg );
  QCOMPARE( loggedMessages[2].message, QString( "critical" ) );
}

void SIGEL_Tools::TST_SIG_Logging::disabledDebugDoesNotEvaluateItsArguments()
{
  int evaluations = 0;

  SIG_Logging::initializeLogging();

  SIG_LOG_DEBUG( "SIGEL_Tools" ) << ++evaluations;

  QCOMPARE( evaluations, 0 );
}

void SIGEL_Tools::TST_SIG_Logging::messageHasTheGivenCategory()
{
  SIG_Logging::initializeLogging();

  SIG_LOG_WARNING( "SIGEL_GP" ) << "warning";
  SIG_LOG_WARNING( "SIGEL_Robot" ) << "warning";

  QCOMPARE( loggedMessages.size(), size_t( 2 ) );
  QCOMPARE( loggedMessages[0].category, QString( "SIGEL_GP" ) );
  QCOMPARE( loggedMessages[1].category, QString( "SIGEL_Robot" ) );
}

void SIGEL_Tools::TST_SIG_Logging::messageIsWrittenWithoutQuotesOrSpaces()
{
  SIG_Logging::initializeLogging();

  SIG_LOG_WARNING( "SIGEL_Tools" ) << "terrain file not found: " << QString( "moon.wrl" ) << ", line " << 3;

  QCOMPARE( loggedMessages.size(), size_t( 1 ) );
  QCOMPARE( loggedMessages[0].message, QString( "terrain file not found: moon.wrl, line 3" ) );
}

void SIGEL_Tools::TST_SIG_Logging::messageKeepsTheFileAndLineOfTheCall()
{
  SIG_Logging::initializeLogging();

  const int line = __LINE__ + 1;
  SIG_LOG_WARNING( "SIGEL_Tools" ) << "warning";

  QCOMPARE( loggedMessages.size(), size_t( 1 ) );
  QCOMPARE( loggedMessages[0].file, QString( __FILE__ ) );
  QCOMPARE( loggedMessages[0].line, line );
}

void SIGEL_Tools::TST_SIG_Logging::everyLineIsDateLevelCategoryAndMessage()
{
  const QString date = "\\[\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}\\.\\d{3}\\]";

  SIG_Logging::initializeLogging();
  QLoggingCategory::setFilterRules( "SIGEL_Tools.debug=true" );

  SIG_LOG_DEBUG( "SIGEL_Tools" ) << "debug message";
  SIG_LOG_INFO( "SIGEL_Tools" ) << "info message";
  SIG_LOG_WARNING( "SIGEL_GP" ) << "warning message";
  SIG_LOG_CRITICAL( "SIGEL_Robot" ) << "critical message";

  QCOMPARE( loggedMessages.size(), size_t( 4 ) );
  QVERIFY2( QRegularExpression( "^" + date + " \\[debug\\] \\[SIGEL_Tools\\] debug message$" ).match( loggedMessages[0].formattedMessage ).hasMatch(), qPrintable( loggedMessages[0].formattedMessage ) );
  QVERIFY2( QRegularExpression( "^" + date + " \\[info\\] \\[SIGEL_Tools\\] info message$" ).match( loggedMessages[1].formattedMessage ).hasMatch(), qPrintable( loggedMessages[1].formattedMessage ) );
  QVERIFY2( QRegularExpression( "^" + date + " \\[warning\\] \\[SIGEL_GP\\] warning message$" ).match( loggedMessages[2].formattedMessage ).hasMatch(), qPrintable( loggedMessages[2].formattedMessage ) );
  QVERIFY2( QRegularExpression( "^" + date + " \\[critical\\] \\[SIGEL_Robot\\] critical message$" ).match( loggedMessages[3].formattedMessage ).hasMatch(), qPrintable( loggedMessages[3].formattedMessage ) );
}

// setFilterRules replaces the rules of initializeLogging, so the rules state "*.debug=false" again.
void SIGEL_Tools::TST_SIG_Logging::ruleForACategoryReachesEveryCallSiteOfThatNameOnly()
{
  SIG_Logging::initializeLogging();
  QLoggingCategory::setFilterRules( "*.debug=false\nSIGEL_GP.debug=true" );

  SIG_LOG_DEBUG( "SIGEL_GP" ) << "first";
  SIG_LOG_DEBUG( "SIGEL_Robot" ) << "robot";
  SIG_LOG_DEBUG( "SIGEL_GP" ) << "second";

  QCOMPARE( loggedMessages.size(), size_t( 2 ) );
  QCOMPARE( loggedMessages[0].message, QString( "first" ) );
  QCOMPARE( loggedMessages[1].message, QString( "second" ) );
}
