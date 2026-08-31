#include "MT_GUI/MT_SearchWidget.h"
#include "MT_GPSystem/MT_Randomizer.h"

#include <QSlider>
#include <QLineEdit>
#include <QSpinBox>
#include <QLCDNumber>
#include <QLayout>

MT_SearchWidget::MT_SearchWidget(QWidget* parent, const char* name, Qt::WindowFlags fl)
: MT_SearchWidgetBase(parent, name, fl), MT_WidgetBase(parent)
{
    powerSpinBox = new DISpinBox( 1, (QWidget*)GroupBox12, "powerSpinBox" );
    lineProbSpinBox = new DISpinBox( 1, (QWidget*)GroupBox12, "lineProbSpinBox" );
    Layout44->addWidget( powerSpinBox, 1, 1 );
    Layout44->addWidget( lineProbSpinBox, 0, 1 );
	powerSpinBox->setRange(1, 0.0, 100.0);
	lineProbSpinBox->setRange(1, 0.0, 100.0);

	validator = new QDoubleValidator(0, 100, 1, this);
	xOverProbEdit->setValidator(validator);
	mutProbEdit->setValidator(validator);
	reproProbEdit->setValidator(validator);

	QObject::connect(xOverProbSlider, SIGNAL(valueChanged(int)), SLOT(slotXOverChanged(int)));
	QObject::connect(mutProbSlider, SIGNAL(valueChanged(int)), SLOT(slotMutChanged(int)));
	QObject::connect(reproProbSlider, SIGNAL(valueChanged(int)), SLOT(slotReproChanged(int)));

	QObject::connect(xOverProbEdit, SIGNAL(textChanged(const QString&)), SLOT(slotXEditChanged(const QString&)));
	QObject::connect(mutProbEdit, SIGNAL(textChanged(const QString&)), SLOT(slotMEditChanged(const QString&)));
	QObject::connect(reproProbEdit, SIGNAL(textChanged(const QString&)), SLOT(slotREditChanged(const QString&)));

	QObject::connect(pt1XOverProbSlider, SIGNAL(valueChanged(int)), SLOT(slotpt1Changed(int)));
	QObject::connect(pt2XOverProbSlider, SIGNAL(valueChanged(int)), SLOT(slotpt2Changed(int)));
	QObject::connect(pt3XOverProbSlider, SIGNAL(valueChanged(int)), SLOT(slotpt3Changed(int)));
}

MT_SearchWidget::~MT_SearchWidget()
{}

/***
 * enables/disables widgets during evolution
 ***/
void MT_SearchWidget::evolutionRunning(bool running)
{
	if(running){

	} else {

	}
}

/***
 * exchange data between GUI and GP-System
 * update GUI
 ***/
void MT_SearchWidget::onShow(MT_GPManager *manager, subst_cache *subst)
{
	QList<double> *probMutPow   = 0;
	QList<double> *probSearchOp = 0;
	QList<double> *probXPoints  = 0;

	manager->getRandomizer()->returnSearchValue(&probMutPow, &probSearchOp, &probXPoints);

	// set the search operator probabilities
	xOverProbSlider->setValue(probSearchOp->at(0));
	mutProbSlider->setValue(probSearchOp->at(1) - probSearchOp->at(0));
	reproProbSlider->setValue(probSearchOp->at(2) - probSearchOp->at(1));

	// set the mutation power
	lineProbSpinBox->setValue(probMutPow->at(0));
	powerSpinBox->setValue(probMutPow->at(1));

	// set the crossover probabilities
	pt1XOverProbSlider->setValue(probXPoints->at(0));
	pt2XOverProbSlider->setValue(probXPoints->at(1) - probXPoints->at(0));
	pt3XOverProbSlider->setValue(probXPoints->at(2) - probXPoints->at(1));
}

/***
 * exchange data between GUI and GP-System
 * update GP-System
 ***/
bool MT_SearchWidget::onHide(MT_GPManager *manager, subst_cache *subst)
{
	QList<double> *probMutPow   = 0;
	QList<double> *probSearchOp = 0;
	QList<double> *probXPoints  = 0;

	manager->getRandomizer()->returnSearchValue(&probMutPow, &probSearchOp, &probXPoints);

	// set the search operator probabilities
	(*probSearchOp)[0] = xOverProbSlider->value();
	(*probSearchOp)[1] = mutProbSlider->value()   + probSearchOp->at(0);
	(*probSearchOp)[2] = reproProbSlider->value() + probSearchOp->at(1);

	// set the crossover probabilities
	(*probMutPow)[0] = lineProbSpinBox->value();
	(*probMutPow)[1] = powerSpinBox->value();

	// set the crossover probabilities
	(*probXPoints)[0] = pt1XOverProbSlider->value();
	(*probXPoints)[1] = pt2XOverProbSlider->value() + probXPoints->at(0);
	(*probXPoints)[2] = pt3XOverProbSlider->value() + probXPoints->at(1);

	return true;
}

/***
 * connect the search operator sliders with the corresponding lineedits and
 * correct the values so that they correctly represent a probability distribution
 ***/
void MT_SearchWidget::slotXOverChanged(int nvalue)
{
	disconnectLEs();
	xOverProbEdit->setText(QString::number((double)nvalue / 10.0));

	int o1, o2;
	int s1 = nvalue;
	int s2 = o1 = mutProbSlider->value();
	int s3 = o2 = reproProbSlider->value();

	adjustProbs(s1, &s2, &s3);

	QObject::disconnect(mutProbSlider, SIGNAL(valueChanged(int)), this, SLOT(slotMutChanged(int)));
	QObject::disconnect(reproProbSlider, SIGNAL(valueChanged(int)), this, SLOT(slotReproChanged(int)));

	if(o1 != s2){ mutProbSlider->setValue(s2); mutProbEdit->setText(QString::number((double)s2 / 10.0)); }
	if(o2 != s3){ reproProbSlider->setValue(s3); reproProbEdit->setText(QString::number((double)s3 / 10.0)); }

	QObject::connect(mutProbSlider, SIGNAL(valueChanged(int)), SLOT(slotMutChanged(int)));
	QObject::connect(reproProbSlider, SIGNAL(valueChanged(int)), SLOT(slotReproChanged(int)));
	reconnectLEs();
}

void MT_SearchWidget::slotMutChanged(int nvalue)
{
	disconnectLEs();
	mutProbEdit->setText(QString::number((double)nvalue / 10.0));

	int o1, o2;
	int s1 = o1 = xOverProbSlider->value();
	int s2 = nvalue;
	int s3 = o2 = reproProbSlider->value();

	adjustProbs(s2, &s3, &s1);

	QObject::disconnect(xOverProbSlider, SIGNAL(valueChanged(int)), this, SLOT(slotXOverChanged(int)));
	QObject::disconnect(reproProbSlider, SIGNAL(valueChanged(int)), this, SLOT(slotReproChanged(int)));

	if(o2 != s3){ reproProbSlider->setValue(s3); reproProbEdit->setText(QString::number((double)s3 / 10.0)); }
	if(o1 != s1){ xOverProbSlider->setValue(s1); xOverProbEdit->setText(QString::number((double)s1 / 10.0)); }

	QObject::connect(xOverProbSlider, SIGNAL(valueChanged(int)), SLOT(slotXOverChanged(int)));
	QObject::connect(reproProbSlider, SIGNAL(valueChanged(int)), SLOT(slotReproChanged(int)));
	reconnectLEs();
}

void MT_SearchWidget::slotReproChanged(int nvalue)
{
	disconnectLEs();
	reproProbEdit->setText(QString::number((double)nvalue / 10.0));

	int o1, o2;
	int s1 = o1 = xOverProbSlider->value();
	int s2 = o2 = mutProbSlider->value();
	int s3 = nvalue;

	adjustProbs(s3, &s1, &s2);

	QObject::disconnect(xOverProbSlider, SIGNAL(valueChanged(int)), this, SLOT(slotXOverChanged(int)));
	QObject::disconnect(mutProbSlider, SIGNAL(valueChanged(int)), this, SLOT(slotMutChanged(int)));

	if(o1 != s1){ xOverProbSlider->setValue(s1); xOverProbEdit->setText(QString::number((double)s1 / 10.0)); }
	if(o2 != s2){ mutProbSlider->setValue(s2); mutProbEdit->setText(QString::number((double)s2 / 10.0)); }

	QObject::connect(xOverProbSlider, SIGNAL(valueChanged(int)), SLOT(slotXOverChanged(int)));
	QObject::connect(mutProbSlider, SIGNAL(valueChanged(int)), SLOT(slotMutChanged(int)));
	reconnectLEs();
}

/***
 * connect the line edit widgets with the corresponding sliders
 ***/
void MT_SearchWidget::slotXEditChanged(const QString &valText){
	double dVal = valText.toDouble();
	int    iVal = (int)(dVal * 10.0);

	xOverProbSlider->setValue(iVal);
}
void MT_SearchWidget::slotMEditChanged(const QString &valText){
	double dVal = valText.toDouble();
	int    iVal = (int)(dVal * 10.0);

	mutProbSlider->setValue(iVal);
}
void MT_SearchWidget::slotREditChanged(const QString &valText){
	double dVal = valText.toDouble();
	int    iVal = (int)(dVal * 10.0);

	reproProbSlider->setValue(iVal);
}

/***
 * connect the XOverPoint slider with the corresponding LCD numbers and
 * correct the values so that they correctly represent a probability distribution
 ***/
void MT_SearchWidget::slotpt1Changed(int nvalue)
{
	pt1XOverProbLCD->display((double)nvalue / 10.0);

	int o1, o2;
	int s1 = nvalue;
	int s2 = o1 = pt2XOverProbSlider->value();
	int s3 = o2 = pt3XOverProbSlider->value();

	adjustProbs(s1, &s2, &s3);

	QObject::disconnect(pt2XOverProbSlider, SIGNAL(valueChanged(int)), this, SLOT(slotpt2Changed(int)));
	QObject::disconnect(pt3XOverProbSlider, SIGNAL(valueChanged(int)), this, SLOT(slotpt3Changed(int)));

	if(o1 != s2){ pt2XOverProbSlider->setValue(s2); pt2XOverProbLCD->display((double)s2 / 10.0); }
	if(o2 != s3){ pt3XOverProbSlider->setValue(s3); pt3XOverProbLCD->display((double)s3 / 10.0); }

	QObject::connect(pt2XOverProbSlider, SIGNAL(valueChanged(int)), SLOT(slotpt2Changed(int)));
	QObject::connect(pt3XOverProbSlider, SIGNAL(valueChanged(int)), SLOT(slotpt3Changed(int)));
}

void MT_SearchWidget::slotpt2Changed(int nvalue)
{
	pt2XOverProbLCD->display((double)nvalue / 10.0);

	int o1, o2;
	int s1 = o1 = pt1XOverProbSlider->value();
	int s2 = nvalue;
	int s3 = o2 = pt3XOverProbSlider->value();

	adjustProbs(s2, &s3, &s1);

	QObject::disconnect(pt1XOverProbSlider, SIGNAL(valueChanged(int)), this, SLOT(slotpt1Changed(int)));
	QObject::disconnect(pt3XOverProbSlider, SIGNAL(valueChanged(int)), this, SLOT(slotpt3Changed(int)));

	if(o2 != s3){ pt3XOverProbSlider->setValue(s3); pt3XOverProbLCD->display((double)s3 / 10.0); }
	if(o1 != s1){ pt1XOverProbSlider->setValue(s1); pt1XOverProbLCD->display((double)s1 / 10.0); }

	QObject::connect(pt1XOverProbSlider, SIGNAL(valueChanged(int)), SLOT(slotpt1Changed(int)));
	QObject::connect(pt3XOverProbSlider, SIGNAL(valueChanged(int)), SLOT(slotpt3Changed(int)));
}

void MT_SearchWidget::slotpt3Changed(int nvalue)
{
	pt3XOverProbLCD->display((double)nvalue / 10.0);

	int o1, o2;
	int s1 = o1 = pt1XOverProbSlider->value();
	int s2 = o2 = pt2XOverProbSlider->value();
	int s3 = nvalue;

	adjustProbs(s3, &s1, &s2);

	QObject::disconnect(pt1XOverProbSlider, SIGNAL(valueChanged(int)), this, SLOT(slotpt1Changed(int)));
	QObject::disconnect(pt2XOverProbSlider, SIGNAL(valueChanged(int)), this, SLOT(slotpt2Changed(int)));

	if(o1 != s1){ pt1XOverProbSlider->setValue(s1); pt1XOverProbLCD->display((double)s1 / 10.0); }
	if(o2 != s2){ pt2XOverProbSlider->setValue(s2); pt2XOverProbLCD->display((double)s2 / 10.0); }

	QObject::connect(pt1XOverProbSlider, SIGNAL(valueChanged(int)), SLOT(slotpt1Changed(int)));
	QObject::connect(pt2XOverProbSlider, SIGNAL(valueChanged(int)), SLOT(slotpt2Changed(int)));
}

/***
 * adjust the values so that they are real probabilities
 * (that means the sum of the 3 values has to be 1000)
 ***/
void MT_SearchWidget::adjustProbs(int s1, int *p2, int *p3)
{
	// get the values
	int s2 = *p2;
	int s3 = *p3;

	int d1, d2;

	d1 = 1000 - (s1+s2+s3);
	d2 = s2 + d1;

	if((0 <= d2) && (d2 <= 1000)){
		s2 = d2;
	} else {
		if(d2 < 0){
			s3 += d2;
			s2 = 0;
		} else {
			s3 += d2-1000;
			s2 = 1000;
		}
	}

	// set the values
	*p2 = s2;
	*p3 = s3;
}

/***
 * functions to avoid endless signal cycling
 * between slider widgets and the corresponding line edit widget
 ***/
void MT_SearchWidget::disconnectLEs()
{
	QObject::disconnect(xOverProbEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotXEditChanged(const QString&)));
	QObject::disconnect(mutProbEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotMEditChanged(const QString&)));
	QObject::disconnect(reproProbEdit, SIGNAL(textChanged(const QString&)), this, SLOT(slotREditChanged(const QString&)));
}
void MT_SearchWidget::reconnectLEs()
{
	QObject::connect(xOverProbEdit, SIGNAL(textChanged(const QString&)), SLOT(slotXEditChanged(const QString&)));
	QObject::connect(mutProbEdit, SIGNAL(textChanged(const QString&)), SLOT(slotMEditChanged(const QString&)));
	QObject::connect(reproProbEdit, SIGNAL(textChanged(const QString&)), SLOT(slotREditChanged(const QString&)));
}