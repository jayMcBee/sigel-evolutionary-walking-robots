#ifndef SIGEL_TOOLS_SIG_DIALOGPARENT_H
#define SIGEL_TOOLS_SIG_DIALOGPARENT_H

#include <QApplication>
#include <QMainWindow>
#include <QWidget>

namespace SIGEL_Tools
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
