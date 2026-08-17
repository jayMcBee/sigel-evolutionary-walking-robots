// MT_WidgetBase.h: Schnittstelle für die Klasse MT_WidgetBase.
//
//////////////////////////////////////////////////////////////////////

#ifndef MT_GUI_WIDGETBASE
#define MT_GUI_WIDGETBASE

#include <qwidget.h>

#include "MT_GPSystem/MT_GPManager.h"
#include "MT_Control/MT_Substitute.h"

class MT_WidgetBase  
{
public:
	MT_WidgetBase(QWidget *parent);
	virtual ~MT_WidgetBase();

	virtual bool onHide(MT_GPManager *manager, subst_cache *subst);
	virtual void onShow(MT_GPManager *manager, subst_cache *subst);

protected:
	QWidget *parentWindow;
};

#endif

