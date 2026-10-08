#include "MT_GUI/MT_WidgetBase.h"

MT_WidgetBase::MT_WidgetBase(QWidget *parent)
{
	parentWindow = parent;
}

void MT_WidgetBase::onShow(MT_GPManager *manager, subst_cache *subst)
{
}

bool MT_WidgetBase::onHide(MT_GPManager *manager, subst_cache *subst)
{
	return true;
}
