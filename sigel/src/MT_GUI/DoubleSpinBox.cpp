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
#include "MT_GUI/DoubleSpinBox.h"
#include "SIGEL_CommonGUI/SIG_WidgetUtils.h"

#include <QLineEdit>
#include <cmath>

DISpinBox::DISpinBox( int decimals, QWidget *parent ) : QSpinBox( parent )
{
	precision = 0;
	dValidator = nullptr;
	iValidator = nullptr;
	iDecimals = decimals;

	dValidator = new QDoubleValidator( this );
	iValidator = new QIntValidator( this );

	SIGEL_CommonGUI::SIG_WidgetUtils::useDotDecimalNumberFormat( this );

	// valueChanged comes only from setValue or when editing ends, not on
	// every keystroke.
	setKeyboardTracking( false );

	if( decimals != 0 )
	{
		// create a DoubleSpinBox
		valueType = MT_SpinBoxValueType::Double;
		precision = pow( 10, decimals );
		dValidator->setRange( 0.0, 100.0, decimals );
		lineEdit()->setValidator( dValidator );
		setRange( 0, dValidator->top() * precision );
		setSingleStep( 10 );
	}
	else
	{
		// create an IntSpinBox
		valueType = MT_SpinBoxValueType::Integer;
		precision = 1;
		iValidator->setRange( 0, 100 );
		lineEdit()->setValidator( iValidator );
		setRange( 0, iValidator->top() );
		setSingleStep( 1 );
	}
}

DISpinBox::~DISpinBox()
{
	delete dValidator;
	delete iValidator;
}

QValidator::State DISpinBox::validate( QString &input, int &pos ) const
{
	const QValidator *v = ( valueType == MT_SpinBoxValueType::Integer ) ? static_cast<const QValidator*>( iValidator )
	                                                                    : static_cast<const QValidator*>( dValidator );
	return v ? v->validate( input, pos ) : QValidator::Acceptable;
}

// Leaves the text alone. QSpinBox::fixup() would rewrite "0.375" to "0".
void DISpinBox::fixup( QString & ) const
{
}

int DISpinBox::valueFromText( const QString &t ) const
{
	if( valueType == MT_SpinBoxValueType::Integer )
	{
		return t.toInt();
	}
	else
	{
		return int( t.toDouble() * precision );
	}
}

QString DISpinBox::textFromValue( int value ) const
{
	if( valueType == MT_SpinBoxValueType::Integer )
	{
		return QString( "%1" ).arg( value );
	}
	else
	{
		return QString::number( value / precision ).append( "." ).append( QString::number( value % precision ).rightJustified( iDecimals, '0' ) );
	}
}

void DISpinBox::setDoubleValue( double value )
{
	setValue( value * precision );
}

void DISpinBox::setIntValue( int value )
{
	setValue( value );
}

double DISpinBox::doubleValue()
{
	return static_cast<double>( value() ) / static_cast<double>( precision );
}

int DISpinBox::intValue()
{
	return value();
}

void DISpinBox::setRange( int minVal, int maxVal )
{
	valueType = MT_SpinBoxValueType::Integer;
	precision = 1;

	setSingleStep( 1 );
	iValidator->setRange( minVal, maxVal );
	lineEdit()->setValidator( iValidator );
	QSpinBox::setRange( minVal, maxVal );
}

void DISpinBox::setRange( int decimals, double minVal, double maxVal )
{
	valueType = MT_SpinBoxValueType::Double;
	iDecimals = decimals;
	precision = pow( 10, decimals );

	setSingleStep( 10 );
	dValidator->setRange( minVal, maxVal, decimals );
	lineEdit()->setValidator( dValidator );
	QSpinBox::setRange( minVal, static_cast<int>( maxVal * precision ) );
}

MT_SpinBoxValueType DISpinBox::getType()
{
	return valueType;
}
