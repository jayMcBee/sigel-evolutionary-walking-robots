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
#ifndef SIGEL_COMMONGUI_SIG_DIALOGPARENT_H
#define SIGEL_COMMONGUI_SIG_DIALOGPARENT_H

#include <QApplication>
#include <QMainWindow>
#include <QWidget>

namespace SIGEL_CommonGUI
{

  // The window a dialog belongs to, for code that holds no window of its own.
  // A dialog with no parent can open behind the main window and block it.
  // The open modal dialog comes first, then the active window. activeWindow()
  // is null for a moment after another dialog closes, so the last choice is
  // the visible main window: SIG_MainWindow, or the slave's simulation window.
  inline QWidget *dialogParent()
  {
    if( !qobject_cast<QApplication *>( QCoreApplication::instance() ) )
      return nullptr;
    if( QWidget *w = QApplication::activeModalWidget() )
      return w;
    if( QWidget *w = QApplication::activeWindow() )
      return w;
    for( QWidget *w : QApplication::topLevelWidgets() )
      if( w->isVisible() && qobject_cast<QMainWindow *>( w ) )
        return w;
    return nullptr;
  }

}

#endif
