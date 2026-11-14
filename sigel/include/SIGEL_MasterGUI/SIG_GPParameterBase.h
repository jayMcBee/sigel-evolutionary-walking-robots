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
  uic makes only the struct Ui::SIG_GPParameterBase. This class inherits it and
  calls setupUi() on itself.

  The custom slots are pure virtual, so SIG_GPParameter must supply each one.
*/
#ifndef SIGEL_MASTERGUI_SIG_GPPARAMETERBASE_H
#define SIGEL_MASTERGUI_SIG_GPPARAMETERBASE_H

#include "ui_SIG_GPParameterBase.h"

#include <QWidget>

class SIG_GPParameterBase : public QWidget, public Ui::SIG_GPParameterBase
{
    Q_OBJECT

public:
    SIG_GPParameterBase( QWidget *parent = nullptr,
                         Qt::WindowFlags fl = Qt::WindowFlags() );

public slots:
    virtual void slotAddHost() = 0;
    virtual void slotChangeGraveyardDir() = 0;
    virtual void slotChangePoolImageDir() = 0;
    virtual void slotCrossoverChanged( int ) = 0;
    virtual void slotDeleteHost() = 0;
    virtual void slotDisableAllHosts() = 0;
    virtual void slotEditHost() = 0;
    virtual void slotEnableAllHosts() = 0;
    virtual void slotMutationChanged( int ) = 0;
    virtual void slotTournamentsPerGenerationChanged( int ) = 0;
};

#endif // SIGEL_MASTERGUI_SIG_GPPARAMETERBASE_H
