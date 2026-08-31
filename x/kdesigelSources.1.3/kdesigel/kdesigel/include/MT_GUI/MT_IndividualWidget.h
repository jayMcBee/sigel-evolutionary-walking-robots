#ifndef MT_GUI_INDIVIDUAL
#define MT_GUI_INDIVIDUAL

#include "MT_GUI/MT_IndividualWidgetBase.h"
#include "MT_GUI/MT_WidgetBase.h"
#include "MT_GUI/MT_AddConstantsWidgetBase.h"
#include "MT_GUI/MT_Editor.h"
#include "MT_GPSystem/MT_Randomizer.h"

#include <QValidator>
#include <QMenu>
#include <QAction>

class MT_IndividualsWidget : public MT_IndividualsWidgetBase, public MT_WidgetBase
{
	Q_OBJECT

	friend class MT_AddConstantsWidget;

public:
	virtual bool onHide(MT_GPManager *manager, subst_cache *subst);
	virtual void onShow(MT_GPManager *manager, subst_cache *subst);
	void evolutionRunning(bool running);

	MT_IndividualsWidget(QWidget* parent=0, const char* name=0, Qt::WindowFlags fl = Qt::WindowFlags());
	~MT_IndividualsWidget();

private slots:
	void slotCreateConstants();
	void slotImportConstants();
	void slotExportConstants();
	void slotChangeSlider(const QString &text);
	void slotChangeEdit(int value);
	void slotEditConst();
	void slotEditConst(QListWidgetItem *item);
	void slotChangeConstant(const QString &ntext);
	void slotRButtonClicked(const QPoint &pos);
	void slotSelectionChanged();
	void slotDelConst();
	void slotResetFocus();
	void slotNumConstChanged();

private:
	double minValue, maxValue;
	bool integer;
	int numToCreate;

	MT_Randomizer *randomizer;

	QIntValidator *validator;
	QMenu	*constContextMenu;
	QAction	*delConstAction;
	QAction	*editConstAction;
	MT_Editor	*editor;

signals:
	void numConstChanged();
};

#endif