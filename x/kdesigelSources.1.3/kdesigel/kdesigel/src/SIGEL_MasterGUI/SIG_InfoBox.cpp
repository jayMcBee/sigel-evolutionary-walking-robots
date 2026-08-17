/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler

  This file is part of Sigel.

  Sigel is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  Sigel is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Sigel; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/
#include <qlayout.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include "SIGEL_MasterGUI/SIG_InfoBox.h"
#include "SIGEL_MasterGUI/SIG_TextView.h"

#include <cstdlib>

SIG_InfoBox::SIG_InfoBox( QWidget *parent, const char *name, bool modal, WFlags f )
  : QDialog( parent, name, modal, f )
{
#ifdef _WINDOWS
  QString sigelRoot( ::getenv( "SIGEL_ROOT" ) );
#else
  QString sigelRoot( std::getenv( "SIGEL_ROOT" ) );
#endif
  this->setCaption( "Sigel InfoBox" );
  QLabel *pixmapLabel = new QLabel( this, "gfxLabel" );
  pixmapLabel->setPixmap( QPixmap( sigelRoot + "/pixmaps/altLogo.png" ) );
  pixmapLabel->setFrameStyle( QFrame::Box | QFrame::Sunken );
  // pixmapLabel->setScaledContents( true );

  QHBoxLayout *hL = new QHBoxLayout( this, 6, -1 );
  hL->addWidget( pixmapLabel );

  QVBoxLayout *vL = new QVBoxLayout( hL, -1, "vLayout" );
  SIG_TextView *theView = new SIG_TextView( this, "SIG_TextView *theView" );
  theView->setVScrollBarMode( QScrollView::AlwaysOff );
  theView->setText("<h3>Sigel v1.1</h3>"
		   "<h3>Developed by PG 368:</h3>"
		   "<ul>"
		   "<li>Christian <b>&quot;Krasstexta&quot;</b> Aue</li>"
		   "<li>Abdeladim <b>&quot;Silent Ad&quot;</b> Benkacem</li>"
		   "<li>Michael <b>&quot;CJ <i>QT</i>&quot;</b> Gregorius</li>"
		   "<li>Andree <b>&quot;MC Overload&quot;</b> Ross</li>"
		   "<li>Abdallah <b>&quot;The Raiyan&quot;</b> Salah Raiyan</li>"
		   "<li>Daniel <b>&quot;Tabmaster Ispell&quot;</b> Sawitzki</li>"
		   "<li>Volker <b>&quot;Alvi-Schnitte&quot;</b> Strunk</li>"
		   "<li>Holger <b>&quot;DJ NOOP&quot;</b> Tuerk</li>"
		   "<li>Chris <b>&quot;MC Royal&quot;</b> Varcol</li>"
		   "</ul><br>"
		   "<h3>Additional programming for v1.x by:</h3>"
		   "<ul>"
		   "<li>Patrick Matters (<tt>matters@ls11.cs.uni-dortmund.de</tt>)</li>"
		   "<li>Jan Barnholt (<tt>jan.barnholt@epost.de</tt>)</li>"
		   "</ul>"
		   );
  vL->addWidget( theView );

  QSpacerItem *okSpacerItem = new QSpacerItem( 20, 20, QSizePolicy::Expanding );
  
  QHBoxLayout *okLayout = new QHBoxLayout( vL, -1, "okLayout" );
  okLayout->addItem( okSpacerItem );

  QPushButton *okPushButton = new QPushButton( "&OK", this, "okPushButton" );
  connect( okPushButton,
	   SIGNAL( clicked() ),
	   this,
	   SLOT( accept() ) );
  okLayout->addWidget( okPushButton );

};

SIG_InfoBox::~SIG_InfoBox()
{

};

