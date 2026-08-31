// MT_Editor.h: Schnittstelle für die Klasse MT_Editor.
//
//////////////////////////////////////////////////////////////////////

#ifndef MT_EDITOR
#define MT_EDITOR

#include <QLineEdit>

class MT_Editor : public QLineEdit  
{
	Q_OBJECT

public:
	MT_Editor(QWidget *parent=0);
	virtual ~MT_Editor();

	void popup(const QRect &rect, const QString &text);

private:
	QString oldText;
	bool	acceptChange;

protected:
	void focusOutEvent(QFocusEvent *event);
	void mousePressEvent(QMouseEvent *event);
	void hideEvent(QHideEvent *event);
	void keyPressEvent(QKeyEvent *event);

signals:
	void newText(const QString &);
	// NOT QLineEdit's signal -- Qt 2.3's QLineEdit had no lostFocus() at all
	// (it arrived in Qt 3). This is MT_Editor's own, emitted from hideEvent on
	// EVERY dismissal, and it survives the port untouched. editingFinished()
	// is not a substitute: it fires on Return as well as focus-out, and the
	// whole point of acceptChange is that Return commits and focus-out does
	// not. Confirmed on the running 1.3.
	void lostFocus();
};

#endif
