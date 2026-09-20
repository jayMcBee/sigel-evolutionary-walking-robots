#ifndef MT_GUI_SEARCHWIDGET
#define MT_GUI_SEARCHWIDGET

#include "MT_GUI/MT_SearchWidgetBase.h"
#include "MT_GUI/MT_WidgetBase.h"
#include "MT_GUI/DoubleSpinBox.h"

#include <QValidator>

class MT_SearchWidget : public MT_SearchWidgetBase, public MT_WidgetBase
{
	Q_OBJECT

public:
	MT_SearchWidget(QWidget* parent=0, const char* name=0, Qt::WindowFlags fl = Qt::WindowFlags());
	~MT_SearchWidget();

	void onShow(MT_GPManager *manager, subst_cache *subst);
	bool onHide(MT_GPManager *manager, subst_cache *subst);
	void evolutionRunning(bool running);

private slots:
	void slotXOverChanged(int nvalue);
	void slotMutChanged(int nvalue);
	void slotReproChanged(int nvalue);

	void slotXEditChanged(const QString &valText);
	void slotMEditChanged(const QString &valText);
	void slotREditChanged(const QString &valText);

	void slotpt1Changed(int nvalue);
	void slotpt2Changed(int nvalue);
	void slotpt3Changed(int nvalue);

private:
	QDoubleValidator *validator;
	DISpinBox *powerSpinBox;
	DISpinBox *lineProbSpinBox;

	void adjustProbs(int s1, int *p2, int *p3);
	void disconnectLEs();
	void reconnectLEs();
};

#endif