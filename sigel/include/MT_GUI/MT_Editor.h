#ifndef MT_EDITOR
#define MT_EDITOR

#include <QLineEdit>

class MT_Editor : public QLineEdit  
{
	Q_OBJECT

public:
	MT_Editor(QWidget *parent=nullptr);

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
	// MT_Editor's own signal, from hideEvent. editingFinished is no substitute:
	// it also fires on Return, and Return must commit where focus-out does not.
	void lostFocus();
};

#endif
