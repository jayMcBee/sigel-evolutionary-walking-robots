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
/****************************************************************************
** Form interface generated from reading ui file 'SIG_AddIndividualsDialogBase.ui'
**
** Created: Thu Mar 22 12:25:56 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef SIG_ADDINDIVIDUALSDIALOG_H
#define SIG_ADDINDIVIDUALSDIALOG_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QLabel;
class QPushButton;
class QSpinBox;

namespace SIGEL_MasterGUI
{
  /**
   * This is the class of the dialog which is used to add individuals to the pool.
   */
class SIG_AddIndividualsDialog : public QDialog
{ 
    Q_OBJECT

public:
    SIG_AddIndividualsDialog( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SIG_AddIndividualsDialog();

    QLabel* textlabelNumber;
    QSpinBox* spinboxNumber;
    QPushButton* pushbuttonOK;
    QPushButton* pushbuttonCancel;

protected:
    QVBoxLayout* SIG_AddIndividualsDialogBaseLayout;
    QHBoxLayout* Layout4;
    QHBoxLayout* Layout1;
};

}

#endif // SIG_ADDINDIVIDUALSDIALOG_H
