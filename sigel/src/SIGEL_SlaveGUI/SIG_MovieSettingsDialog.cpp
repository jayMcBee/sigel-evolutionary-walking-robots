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
#include "SIGEL_SlaveGUI/SIG_MovieSettingsDialog.h"
#include <QFileDialog>
#include <QLabel>
#include <QSpinBox>

namespace SIGEL_SlaveGUI
{

/* 
 *  Constructs a SIG_MovieSettingsDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
SIG_MovieSettingsDialog::SIG_MovieSettingsDialog( int imgWidth, int imgHeight, QWidget* parent,  const char* name, bool modal, Qt::WindowFlags fl )
    : SIG_MovieSettingsDialogBase( parent, name, modal, fl )
{
	scrWidth  = imgWidth;
	scrHeight = imgHeight;
	ratio 	 = (double)scrWidth / (double)scrHeight;
	QString num;
	textlabelCurScreenWidth->setText(num.setNum(imgWidth));
	textlabelCurScreenHeight->setText(num.setNum(imgHeight));
};

/*  
 *  Destroys the object and frees any allocated resources
 */
SIG_MovieSettingsDialog::~SIG_MovieSettingsDialog()
{
    // no need to delete child widgets, Qt does it all for us
};

/* 
 * public slot.
 */
void SIG_MovieSettingsDialog::slotToolButtonClicked()
{
  QString newDirectory =
    QFileDialog::getExistingDirectory( this,
				       "Select Movie Directory",
				       lineeditDirectory->text(),
				       QFileDialog::ShowDirsOnly );
  if( !newDirectory.isNull() )
    lineeditDirectory->setText( newDirectory );
};

void SIG_MovieSettingsDialog::slotChangedAspectRatio(bool on)
{
	if(on){
		connect( (QObject *)spinboxWidth, SIGNAL( valueChanged(int) ), this, SLOT( slotSetHeight(int) ) );
		connect( (QObject *)spinboxHeight, SIGNAL( valueChanged(int) ), this, SLOT( slotSetWidth(int) ) );
	} else {
		disconnect( (QObject *)spinboxWidth, SIGNAL( valueChanged(int) ), this, SLOT( slotSetHeight(int) ) );
		disconnect( (QObject *)spinboxHeight, SIGNAL( valueChanged(int) ), this, SLOT( slotSetWidth(int) ) );
	}
};

void SIG_MovieSettingsDialog::slotSetHeight(int nWidth)
{
	disconnect( (QObject *)spinboxHeight, SIGNAL( valueChanged(int) ), this, SLOT( slotSetWidth(int) ) );
	spinboxHeight->setValue( (int) ((double) nWidth / ratio));
	connect( (QObject *)spinboxHeight, SIGNAL( valueChanged(int) ), this, SLOT( slotSetWidth(int) ) );
};

void SIG_MovieSettingsDialog::slotSetWidth(int nHeight)
{
	disconnect( (QObject *)spinboxWidth, SIGNAL( valueChanged(int) ), this, SLOT( slotSetHeight(int) ) );
	spinboxWidth->setValue( (int) ((double) nHeight * ratio));
	connect( (QObject *)spinboxWidth, SIGNAL( valueChanged(int) ), this, SLOT( slotSetHeight(int) ) );
};

}
