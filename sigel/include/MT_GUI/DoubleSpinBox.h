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
#ifndef MT_GUI_DOUBLESPINBOX_H
#define MT_GUI_DOUBLESPINBOX_H

#include <QSpinBox>
#include <QValidator>
#include "MT_GUI/MT_SpinBoxValueType.h"

class DISpinBox : public QSpinBox
{
public:
	DISpinBox( int decimals, QWidget *parent = nullptr );
	~DISpinBox() override;

	MT_SpinBoxValueType getType();

	void setDoubleValue( double value );
	double doubleValue();

	void setIntValue( int value );
	int intValue();

	void setRange( int decimals, double minVal, double maxVal );
	void setRange( int minVal, int maxVal );

private:
	QString textFromValue( int value ) const override;
	int valueFromText( const QString &t ) const override;

	// QSpinBox's own validate is an integer parser and runs before
	// valueFromText, so "0.375" would be rewritten to "0".
	QValidator::State validate( QString &input, int &pos ) const override;
	void fixup( QString &input ) const override;

	MT_SpinBoxValueType valueType;
	int precision;
	int iDecimals;
	QIntValidator *iValidator;
	QDoubleValidator *dValidator;
};

#endif // MT_GUI_DOUBLESPINBOX_H
