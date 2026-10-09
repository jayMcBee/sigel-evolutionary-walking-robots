#ifndef MT_GUI_WIDGETBASE
#define MT_GUI_WIDGETBASE

#include <QWidget>

#include "MT_GPSystem/MT_GPManager.h"
#include "MT_Control/MT_EstimationState.h"
#include "MT_Control/MT_Substitute.h"

class MT_WidgetBase  
{
public:
	MT_WidgetBase(QWidget *parent);
	virtual ~MT_WidgetBase() = default;

	virtual bool onHide(MT_GPManager *manager, MT_EstimationState *subst);
	virtual void onShow(MT_GPManager *manager, MT_EstimationState *subst);

protected:
	QWidget *parentWindow;
};

#endif
