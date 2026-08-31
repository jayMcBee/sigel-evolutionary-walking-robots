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
#include <QTextBrowser>
#include <QMouseEvent>

class SIG_TextView : public QTextBrowser
{
  Q_OBJECT

 public:
  SIG_TextView( QWidget *parent = 0, const char *name = 0 );
  ~SIG_TextView();

 protected:
  // Qt 2 QScrollView called the three handlers below itself; Qt 6 delivers
  // every viewport event through viewportEvent() instead.
  bool viewportEvent( QEvent *e ) override;

  void viewportMousePressEvent( QMouseEvent * );
  void viewportMouseReleaseEvent( QMouseEvent * );
  void viewportMouseMoveEvent( QMouseEvent * );

  /**
   * Saves the last point where a mouse event occured.
   */
  int oldY;

  /**
   * Indicates whether to scroll up (false) or down(true).
   */
  bool scrollDown;

  /**
   * Indicates autoscroll on or off.
   */
  bool scroll;

 private slots:
  void updateScroll();
};


