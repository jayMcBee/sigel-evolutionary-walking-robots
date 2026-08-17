// MT_WidgetBase.cpp: Implementierung der Klasse MT_WidgetBase.
//
//////////////////////////////////////////////////////////////////////

#include "MT_GUI/MT_WidgetBase.h"

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

MT_WidgetBase::MT_WidgetBase(QWidget *parent)
{
	parentWindow = parent;
}

MT_WidgetBase::~MT_WidgetBase()
{

}

void MT_WidgetBase::onShow(MT_GPManager *manager, subst_cache *subst)
{

}

bool MT_WidgetBase::onHide(MT_GPManager *manager, subst_cache *subst)
{
	return true;
}
