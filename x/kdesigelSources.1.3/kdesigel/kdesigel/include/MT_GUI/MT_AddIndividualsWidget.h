/****************************************************************************
** Form interface generated from reading ui file 'MT_AddIndividualsWidget.ui'
**
** Created: Tue Mar 25 16:05:10 2003
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef MT_ADDINDIVIDUALSWIDGETBASE_H
#define MT_ADDINDIVIDUALSWIDGETBASE_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QLabel;
class QPushButton;
class QSpinBox;

class MT_AddIndividualsWidgetBase : public QDialog
{ 
    Q_OBJECT

public:
    MT_AddIndividualsWidgetBase( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~MT_AddIndividualsWidgetBase();

    QLabel* TextLabel1;
    QSpinBox* spinboxNumber;
    QPushButton* buttonOk;
    QPushButton* buttonCancel;

protected:
    QGridLayout* MT_AddIndividualsWidgetBaseLayout;
    QHBoxLayout* Layout1;
};

#endif // MT_ADDINDIVIDUALSWIDGETBASE_H
