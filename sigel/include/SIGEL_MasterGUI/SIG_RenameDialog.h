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
** Form interface generated from reading ui file 'SIG_RenameDialog.ui'
**
** Created: Thu May 17 16:23:12 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef SIGEL_MASTERGUI_SIG_RENAMEDIALOG_H
#define SIGEL_MASTERGUI_SIG_RENAMEDIALOG_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QLabel;
class QLineEdit;
class QPushButton;

namespace SIGEL_MasterGUI
{
  /**
   * This is the class of the dialog which is used to rename the experiment.
   */
class SIG_RenameDialog : public QDialog
{ 
    Q_OBJECT

public:
    SIG_RenameDialog( QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags() );
    ~SIG_RenameDialog();

    QLabel* textlabelNewName;
    QLineEdit* lineeditNewName;
    QPushButton* pushbuttonOK;
    QPushButton* pushbuttonCancel;

protected:
    QVBoxLayout* SIG_RenameDialogLayout;
    QHBoxLayout* Layout2;
    QHBoxLayout* Layout3;
};

}

#endif // SIGEL_MASTERGUI_SIG_RENAMEDIALOG_H
