// MT_Editor.h: Schnittstelle für die Klasse MT_Editor.
//
//////////////////////////////////////////////////////////////////////

#ifndef MT_EDITOR
#define MT_EDITOR

#include <qlineedit.h>

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
	void lostFocus();
};

#endif
