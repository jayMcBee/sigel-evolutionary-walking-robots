// MT_Editor.cpp: Implementierung der Klasse MT_Editor.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GUI/MT_Editor.h"

#include <qapplication.h>

MT_Editor::MT_Editor(QWidget *parent) : QLineEdit(parent, "MT_Editor")
{
	setFrame(false);
	hide();
}

MT_Editor::~MT_Editor()
{

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
/*		QMouseEvent me(QEvent::MouseButtonPress, event->pos(), 
			event->globalPos(), event->button(),
			event->state());
		QWidget *receiver = QApplication::widgetAt(event->globalPos(), true);
		if(receiver){
			QApplication::sendEvent(receiver, &me);
		}*/
		hide();
	}
}

void MT_Editor::keyPressEvent(QKeyEvent *event)
{
	switch(event->key()){
	case Key_Escape: 
		hide();
		break;
	case Key_Enter: 
	case Key_Return: 
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