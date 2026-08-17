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
#ifndef SIGEL_SLAVEGUI_SIG_MOVIESETTINGSDIALOG_H
#define SIGEL_SLAVEGUI_SIG_MOVIESETTINGSDIALOG_H
#include "SIGEL_SlaveGUI/SIG_MovieSettingsDialogBase.h"

namespace SIGEL_SlaveGUI
{

class SIG_MovieSettingsDialog : public SIG_MovieSettingsDialogBase
{ 
    Q_OBJECT

public:
    SIG_MovieSettingsDialog( int imgWidth, int imgHeight, QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~SIG_MovieSettingsDialog();

public slots:
    void slotToolButtonClicked();
    void slotSetHeight(int nWidth);
    void slotSetWidth(int nHeight);
    void slotChangedAspectRatio(bool on);

private:
	int scrWidth;
	int scrHeight;
	double ratio;

};

}

#endif // SIGEL_SLAVEGUI_SIG_MOVIESETTINGSDIALOG_H
