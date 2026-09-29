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
#include "SIGEL_SlaveGUI/SIG_MovieSettingsDialog.h"
#include "SIGEL_SlaveGUI/SIG_SimulationVisualisationWidget.h"
#include <QFileDialog>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>
#include <QScreen>
#include <QEvent>

namespace SIGEL_SlaveGUI
{

/* 
 *  Constructs a SIG_MovieSettingsDialog which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
SIG_MovieSettingsDialog::SIG_MovieSettingsDialog( QWidget *view, double stepSize, QWidget* parent,  const char* name, bool modal, Qt::WindowFlags fl )
    : SIG_MovieSettingsDialogBase( parent, name, modal, fl ),
      view( view ),
      stepSize( stepSize )
{
	textlabelFrameFit->setForegroundRole( QPalette::PlaceholderText );
	textlabelStepsPerFrame->setForegroundRole( QPalette::PlaceholderText );

	// A maximised window keeps its size, so the resize would do nothing.
	if ( view->window()->isMaximized() || view->window()->isFullScreen() )
	  {
	    pushbuttonResizeViewToMatch->setEnabled( false );
	    pushbuttonResizeViewToMatch->setToolTip( "The window is maximised; restore it to resize it." );
	  }

	connect( spinboxWidth, SIGNAL( valueChanged(int) ), this, SLOT( slotUpdateSizeLabels() ) );
	connect( spinboxHeight, SIGNAL( valueChanged(int) ), this, SLOT( slotUpdateSizeLabels() ) );
	connect( pushbuttonViewSizeToMovie, SIGNAL( clicked() ), this, SLOT( slotViewSizeToMovie() ) );
	connect( pushbuttonResizeViewToMatch, SIGNAL( clicked() ), this, SLOT( slotResizeViewToMatch() ) );
	connect( spinboxFrameRate, SIGNAL( valueChanged(int) ), this, SLOT( slotUpdateStepsPerFrame() ) );

	// The window manager resizes the window later, so the labels follow
	// the view's own resize events.
	view->installEventFilter( this );
	slotUpdateSizeLabels();
	slotUpdateStepsPerFrame();
};

void SIG_MovieSettingsDialog::setSettings( SIG_MovieSettings const &settings )
{
  spinboxWidth->setValue( settings.width );
  spinboxHeight->setValue( settings.height );
  spinboxFrameRate->setValue( settings.frameRate );
  lineeditDirectory->setText( settings.directory );
  lineeditFilePrefix->setText( settings.filePrefix );
  spinboxMaxFrames->setValue( settings.maxFrames );
  comboboxFormat->setCurrentIndex( comboboxFormat->findText( settings.format.toUpper() ) );
  spinboxQuality->setValue( settings.quality );
  checkboxUseLeadingZeros->setChecked( settings.useLeadingZeros );
};

SIG_MovieSettings SIG_MovieSettingsDialog::settings() const
{
  SIG_MovieSettings settings;
  settings.width = spinboxWidth->value();
  settings.height = spinboxHeight->value();
  settings.frameRate = spinboxFrameRate->value();
  settings.directory = lineeditDirectory->text();
  if( settings.directory.right(1) != "/" )
    settings.directory.append( "/" );
  settings.filePrefix = lineeditFilePrefix->text();
  settings.format = comboboxFormat->currentText().toLower();
  settings.maxFrames = spinboxMaxFrames->value();
  settings.quality = spinboxQuality->value();
  settings.useLeadingZeros = checkboxUseLeadingZeros->isChecked();
  return settings;
};

/* 
 * public slot.
 */
void SIG_MovieSettingsDialog::slotToolButtonClicked()
{
  QString newDirectory =
    QFileDialog::getExistingDirectory( this,
				       "Select Movie Directory",
				       lineeditDirectory->text(),
				       QFileDialog::ShowDirsOnly );
  if( !newDirectory.isNull() )
    lineeditDirectory->setText( newDirectory );
};

bool SIG_MovieSettingsDialog::eventFilter( QObject *watched, QEvent *event )
{
  if ( watched == view && event->type() == QEvent::Resize )
    slotUpdateSizeLabels();
  return SIG_MovieSettingsDialogBase::eventFilter( watched, event );
};

QSize SIG_MovieSettingsDialog::viewSize() const
{
  return QSize( qRound( view->width() * view->devicePixelRatioF() ),
		qRound( view->height() * view->devicePixelRatioF() ) );
};

/*
 * Shows the view size, and whether a frame of the output size is cut from
 * the view, padded around it, or both.
 */
void SIG_MovieSettingsDialog::slotUpdateSizeLabels()
{
  QSize current = viewSize();
  textlabelViewSize->setText( QString( "%1 \u00D7 %2" ).arg( current.width() ).arg( current.height() ) );

  bool clipped = current.width() > spinboxWidth->value()
    || current.height() > spinboxHeight->value();
  bool letterboxed = current.width() < spinboxWidth->value()
    || current.height() < spinboxHeight->value();

  if ( clipped && letterboxed )
    textlabelFrameFit->setText( "Rendered frames will be clipped and letterboxed." );
  else if ( clipped )
    textlabelFrameFit->setText( "Rendered frames will be clipped." );
  else if ( letterboxed )
    textlabelFrameFit->setText( "Rendered frames will be letterboxed." );
  else
    textlabelFrameFit->clear();
};

/*
 * Shows how many simulation steps a frame covers at the chosen frame rate,
 * and the rate that gives.
 */
void SIG_MovieSettingsDialog::slotUpdateStepsPerFrame()
{
  int steps = SIG_SimulationVisualisationWidget::stepsPerFrame( stepSize, spinboxFrameRate->value() );
  QString rate = QString::number( 1.0 / ( stepSize * steps ), 'f', 2 );

  if ( steps == 1 )
    textlabelStepsPerFrame->setText( "One frame every simulation step, " + rate + " fps." );
  else
    textlabelStepsPerFrame->setText( QString( "One frame every %1 simulation steps, " ).arg( steps ) + rate + " fps." );
};

void SIG_MovieSettingsDialog::slotViewSizeToMovie()
{
  QSize current = viewSize();
  spinboxWidth->setValue( current.width() );
  spinboxHeight->setValue( current.height() );
};

/*
 * Grows or shrinks the window by the difference between the output size and
 * the view size, within the screen's available area and the window's minimum
 * size, and keeps the window on the screen.
 */
void SIG_MovieSettingsDialog::slotResizeViewToMatch()
{
  QWidget *window = view->window();
  qreal ratio = view->devicePixelRatioF();
  QSize change( qRound( spinboxWidth->value() / ratio ) - view->width(),
		qRound( spinboxHeight->value() / ratio ) - view->height() );

  QRect available = window->screen()->availableGeometry();
  QSize frameExtra = window->frameGeometry().size() - window->size();
  QSize target = ( window->size() + change )
    .boundedTo( available.size() - frameExtra )
    .expandedTo( window->minimumSize() );

  // The window's minimum size can exceed the available area; the window then
  // goes to the area's top-left corner.
  QSize frameSize = target + frameExtra;
  int rightmost = available.right() + 1 - frameSize.width();
  int lowest = available.bottom() + 1 - frameSize.height();

  int x = qMin( window->frameGeometry().left(), rightmost );
  int y = qMin( window->frameGeometry().top(), lowest );
  QPoint position( qMax( available.left(), x ),
		   qMax( available.top(), y ) );

  window->resize( target );
  window->move( position );
};

}
