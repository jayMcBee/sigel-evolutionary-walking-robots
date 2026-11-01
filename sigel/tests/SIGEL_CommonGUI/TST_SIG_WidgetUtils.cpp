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
#include "SIGEL_CommonGUI/TST_SIG_WidgetUtils.h"

#include "SIGEL_CommonGUI/SIG_WidgetUtils.h"

#include <QDoubleValidator>
#include <QIntValidator>
#include <QtTest>

void SIGEL_CommonGUI::TST_SIG_WidgetUtils::initTestCase_data()
{
	QTest::addColumn<QLocale>( "defaultLocale" );

	QTest::newRow( "C" ) << QLocale::c();
	QTest::newRow( "German" ) << QLocale( QLocale::German, QLocale::Germany );
	QTest::newRow( "English" ) << QLocale( QLocale::English, QLocale::UnitedStates );
}

void SIGEL_CommonGUI::TST_SIG_WidgetUtils::init()
{
	QFETCH_GLOBAL( QLocale, defaultLocale );

	previousDefaultLocale = QLocale();
	QLocale::setDefault( defaultLocale );
}

void SIGEL_CommonGUI::TST_SIG_WidgetUtils::cleanup()
{
	QLocale::setDefault( previousDefaultLocale );
}

void SIGEL_CommonGUI::TST_SIG_WidgetUtils::intValidatorAcceptsTheTextToIntReads_data()
{
	QTest::addColumn<QString>( "text" );
	QTest::addColumn<int>( "value" );

	QTest::newRow( "without a separator" ) << "1000" << 1000;
	QTest::newRow( "negative" ) << "-5" << -5;
}

void SIGEL_CommonGUI::TST_SIG_WidgetUtils::intValidatorAcceptsTheTextToIntReads()
{
	QFETCH( QString, text );
	QFETCH( int, value );
	QObject owner;
	QIntValidator validator( &owner );
	int position = 0;

	SIG_WidgetUtils::useDotDecimalNumberFormat( &owner );

	QCOMPARE( validator.validate( text, position ), QValidator::Acceptable );
	QCOMPARE( text.toInt(), value );
}

void SIGEL_CommonGUI::TST_SIG_WidgetUtils::intValidatorRefusesTheTextToIntCannotRead_data()
{
	QTest::addColumn<QString>( "text" );

	QTest::newRow( "comma as thousands separator" ) << "1,000";
	QTest::newRow( "two thousands separators" ) << "1,000,000";
	QTest::newRow( "dot as thousands separator" ) << "1.000";
}

void SIGEL_CommonGUI::TST_SIG_WidgetUtils::intValidatorRefusesTheTextToIntCannotRead()
{
	QFETCH( QString, text );
	QObject owner;
	QIntValidator validator( &owner );
	int position = 0;
	bool readable = true;

	SIG_WidgetUtils::useDotDecimalNumberFormat( &owner );
	text.toInt( &readable );

	QVERIFY( !readable );
	QCOMPARE( validator.validate( text, position ), QValidator::Invalid );
}

void SIGEL_CommonGUI::TST_SIG_WidgetUtils::doubleValidatorAcceptsTheTextToDoubleReads_data()
{
	QTest::addColumn<QString>( "text" );
	QTest::addColumn<double>( "value" );

	QTest::newRow( "dot as decimal point" ) << "0.375" << 0.375;
	QTest::newRow( "dot after thousands" ) << "1000.5" << 1000.5;
	QTest::newRow( "negative" ) << "-2.5" << -2.5;
	QTest::newRow( "exponent" ) << "1e3" << 1000.0;
}

void SIGEL_CommonGUI::TST_SIG_WidgetUtils::doubleValidatorAcceptsTheTextToDoubleReads()
{
	QFETCH( QString, text );
	QFETCH( double, value );
	QObject owner;
	QDoubleValidator validator( &owner );
	int position = 0;

	SIG_WidgetUtils::useDotDecimalNumberFormat( &owner );

	QCOMPARE( validator.validate( text, position ), QValidator::Acceptable );
	QCOMPARE( text.toDouble(), value );
}

void SIGEL_CommonGUI::TST_SIG_WidgetUtils::doubleValidatorRefusesTheTextToDoubleCannotRead_data()
{
	QTest::addColumn<QString>( "text" );

	QTest::newRow( "comma as decimal point" ) << "0,375";
	QTest::newRow( "comma as thousands separator" ) << "1,000";
	QTest::newRow( "comma and dot" ) << "1,000.5";
	QTest::newRow( "dot and comma" ) << "1.000,5";
}

void SIGEL_CommonGUI::TST_SIG_WidgetUtils::doubleValidatorRefusesTheTextToDoubleCannotRead()
{
	QFETCH( QString, text );
	QObject owner;
	QDoubleValidator validator( &owner );
	int position = 0;
	bool readable = true;

	SIG_WidgetUtils::useDotDecimalNumberFormat( &owner );
	text.toDouble( &readable );

	QVERIFY( !readable );
	QCOMPARE( validator.validate( text, position ), QValidator::Invalid );
}

void SIGEL_CommonGUI::TST_SIG_WidgetUtils::validatorsOfBothKindsAtEveryDepthUnderTheOwnerRefuseAThousandsSeparator()
{
	QObject owner;
	QObject child( &owner );
	QObject grandchild( &child );
	QIntValidator directIntValidator( &owner );
	QDoubleValidator directDoubleValidator( &owner );
	QIntValidator deepIntValidator( &grandchild );
	QDoubleValidator deepDoubleValidator( &grandchild );
	QString text( "1,000" );
	int position = 0;

	SIG_WidgetUtils::useDotDecimalNumberFormat( &owner );

	QCOMPARE( directIntValidator.validate( text, position ), QValidator::Invalid );
	QCOMPARE( directDoubleValidator.validate( text, position ), QValidator::Invalid );
	QCOMPARE( deepIntValidator.validate( text, position ), QValidator::Invalid );
	QCOMPARE( deepDoubleValidator.validate( text, position ), QValidator::Invalid );
}

void SIGEL_CommonGUI::TST_SIG_WidgetUtils::validatorOutsideTheOwnerKeepsItsLocale()
{
	QObject parent;
	QObject owner( &parent );
	QIntValidator validatorOutside( &parent );
	QLocale localeBefore = validatorOutside.locale();

	SIG_WidgetUtils::useDotDecimalNumberFormat( &owner );

	QVERIFY( validatorOutside.locale() == localeBefore );
}

void SIGEL_CommonGUI::TST_SIG_WidgetUtils::defaultLocaleStaysTheSame()
{
	QFETCH_GLOBAL( QLocale, defaultLocale );
	QObject owner;
	QIntValidator validator( &owner );

	SIG_WidgetUtils::useDotDecimalNumberFormat( &owner );

	QVERIFY( QLocale() == defaultLocale );
}
