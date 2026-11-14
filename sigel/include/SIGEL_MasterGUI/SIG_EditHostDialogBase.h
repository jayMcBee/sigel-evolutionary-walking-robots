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
/*
  uic makes only the struct Ui::SIG_EditHostDialogBase. This class inherits it
  and calls setupUi() on itself.
*/
#ifndef SIGEL_MASTERGUI_SIG_EDITHOSTDIALOGBASE_H
#define SIGEL_MASTERGUI_SIG_EDITHOSTDIALOGBASE_H

#include "ui_SIG_EditHostDialogBase.h"

#include <QDialog>

class SIG_EditHostDialogBase : public QDialog, public Ui::SIG_EditHostDialogBase
{
    Q_OBJECT

public:
    SIG_EditHostDialogBase(QWidget* parent = nullptr, bool modal = false, Qt::WindowFlags fl = Qt::WindowFlags());

public slots:
    virtual void slotToolbuttonSlaveDirectoryClicked() = 0;
};

#endif // SIGEL_MASTERGUI_SIG_EDITHOSTDIALOGBASE_H
