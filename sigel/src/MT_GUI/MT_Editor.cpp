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
#include "MT_GUI/MT_Editor.h"

#include <QApplication>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QHideEvent>
#include <QFocusEvent>

MT_Editor::MT_Editor(QWidget *parent) : QLineEdit(parent)
{
	setObjectName( QString::fromUtf8( "MT_Editor" ) );
	setFrame(false);
	hide();
}

void MT_Editor::popup(const QRect &rect, const QString &text)
{
	oldText = text;
	acceptChange = false;
	setGeometry(rect);
	setText(text);
	selectAll();
	show();
	setFocus();
	grabMouse();
}

void MT_Editor::focusOutEvent(QFocusEvent *event)
{
	hide();
}

void MT_Editor::mousePressEvent(QMouseEvent *event)
{
	if(! rect().contains(event->pos())){
		hide();
	}
}

void MT_Editor::keyPressEvent(QKeyEvent *event)
{
	switch(event->key()){
	case Qt::Key_Escape: 
		hide();
		break;
	case Qt::Key_Enter: 
	case Qt::Key_Return: 
		acceptChange = true;
		hide();
		break;
	default:
		QLineEdit::keyPressEvent(event);
		break;
	}
}

void MT_Editor::hideEvent(QHideEvent *event)
{
	releaseMouse();
	if(oldText != text() && acceptChange){
		emit newText(text());
	}
	emit lostFocus();
}