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
#include "SIGEL_MasterGUI/SIG_TextView.h"
#include <qtimer.h>

SIG_TextView::SIG_TextView( QWidget *parent, const char *name )
  : QTextView( parent, name ), oldY(0), scrollDown( true ), scroll( true )
{
  QTimer *scrollTimer = new QTimer( this );
  connect( scrollTimer,
	   SIGNAL( timeout() ),
	   this,
	   SLOT( updateScroll() ) );
  scrollTimer->start( 100 );
};

SIG_TextView::~SIG_TextView()
{

};

void SIG_TextView::viewportMousePressEvent( QMouseEvent *e )
{
  oldY = e->y();
  scroll = false;
};

void SIG_TextView::viewportMouseReleaseEvent( QMouseEvent * )
{
  scroll = true;
};

void SIG_TextView::viewportMouseMoveEvent( QMouseEvent *e )
{
  int newPos = e->y();
  int toMove = oldY - newPos;
  scrollBy( 0, toMove );
  oldY = newPos;
};

void SIG_TextView::updateScroll()
{
  if( scroll )
    {
      int cHeight = contentsHeight();
      int cY = contentsY();
      int vHeight = visibleHeight();
      if( scrollDown )
	{
	  if ( cY + vHeight >= cHeight )
	    scrollDown = false;
	  else
	    scrollDown = true;
	}
      else
	{
	  if( cY == 0 )
	    scrollDown = true;
	  else
	    scrollDown = false;
	}
      if( scrollDown )
	scrollBy( 0, 1 );
      else
	scrollBy( 0, -1 );
    }
};
