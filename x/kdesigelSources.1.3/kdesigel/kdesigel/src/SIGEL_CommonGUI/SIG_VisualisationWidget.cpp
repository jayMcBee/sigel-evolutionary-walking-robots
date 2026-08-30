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
#include <QMouseEvent>
#include "SIGEL_CommonGUI/SIG_VisualisationWidget.h"

#include <cmath>

//namespace SIGEL_CommonGUI		// The visual c++ compiler gets in trouble with the
//{										// corresponding moc file if this namespace is used
											// and moc ignore preprocessor statements.


  SIG_VisualisationWidget::SIG_VisualisationWidget( QWidget *parent,
						    char const *name,
						    Qt::WindowFlags f )
    : QOpenGLWidget(parent, f),
      visualisation(0),
      floatingTextWidgets(),
      floatingTextsSize(),
      showAncorPointsState(0),
      yaw(0),
      pitch(30),
      distance(2),
      mouseXPos(0),
      mouseYPos(0),
      automaticRefresh(true),
      mouseSensity(0.5),
#ifdef _WINDOWS
		pi( ::atan(1) * 4 )
#else
      pi( std::atan(1) * 4 )
#endif
  {
    if ( name )
      setObjectName( QString::fromUtf8( name ) );
  };

  SIG_VisualisationWidget::~SIG_VisualisationWidget()
  {
    delete visualisation;

    // The labels are Qt CHILDREN of this widget and were ALSO owned by the
    // Qt 2 vector's autoDelete. That was not a double free: the member is
    // destroyed before the QWidget base, so each label unparented itself and
    // ~QWidget then found no children left. Disassembly of the 1.3 slave shows
    // exactly that order: ~QVector at 0x0806edec, then ~QGLWidget at
    // 0x0806edfc. This body runs before the base destructor, so the order
    // is preserved.
    qDeleteAll( floatingTextWidgets );
    floatingTextWidgets.clear();
  };

  void SIG_VisualisationWidget::setRenderMode( const QString & string )
  {
    if (visualisation)
      {
	if (string == "Wireframe")
	  visualisation->viewSettings.renderMode = SIGEL_Visualisation::SIG_ViewSettings::wireFrame;
	else if (string == "Flatshaded")
	  visualisation->viewSettings.renderMode = SIGEL_Visualisation::SIG_ViewSettings::flatShaded;
	else if (string == "Gouraudshaded")
	  visualisation->viewSettings.renderMode = SIGEL_Visualisation::SIG_ViewSettings::garoudShaded;
	if (automaticRefresh)
	  update();
      };
  };

  void SIG_VisualisationWidget::setAmbientLighting( int newValue )
  {
    if (visualisation)
      {
	double const ambientSceneColor = double( newValue ) / 100;

	visualisation->setAmbientSceneColor( ambientSceneColor,
					     ambientSceneColor,
					     ambientSceneColor );

	if (automaticRefresh)
	  update();
      };
  };

  void SIG_VisualisationWidget::setShowAncorPoints( int state )
  {
    if ( (showAncorPointsState < 2) && (state == 2) )
      for ( int i = 0; i < floatingTextWidgets.size(); i++ )
	floatingTextWidgets[i]->show();

    if ( (showAncorPointsState == 2) && (state < 2) )
      for ( int i = 0; i < floatingTextWidgets.size(); i++ )
	floatingTextWidgets[i]->hide();

    showAncorPointsState = state;

    update();
  };

  void SIG_VisualisationWidget::setYaw( double yyaw )
  {
    yaw = yyaw;
    updateEyePoint();
  };

  void SIG_VisualisationWidget::setPitch( double ppitch )
  {
    pitch = ppitch;
    updateEyePoint();
  };

  void SIG_VisualisationWidget::setDistance( double ddistance )
  {
    distance = ddistance;
    updateEyePoint();
  };

  void SIG_VisualisationWidget::updateEyePoint()
  {
    if (visualisation)
      {
	double const radEyeYaw = (yaw / 360) * 2 * pi;
	double const radEyePitch = (pitch / 360) * 2 * pi;

#ifdef _WINDOWS
	double const sinEyeYaw = ::sin(radEyeYaw);
	double const cosEyeYaw = ::cos(radEyeYaw);
	double const sinEyePitch = ::sin(radEyePitch);
	double const cosEyePitch = ::cos(radEyePitch);
#else
	double const sinEyeYaw = std::sin(radEyeYaw);
	double const cosEyeYaw = std::cos(radEyeYaw);
	double const sinEyePitch = std::sin(radEyePitch);
	double const cosEyePitch = std::cos(radEyePitch);
#endif

	double eyeX = sinEyeYaw * cosEyePitch * distance;
	double eyeY = sinEyePitch * distance;
	double eyeZ = cosEyeYaw * cosEyePitch * distance;

	visualisation->viewSettings.eyePoint.set(0, eyeX);
	visualisation->viewSettings.eyePoint.set(1, eyeY);
	visualisation->viewSettings.eyePoint.set(2, eyeZ);
	visualisation->viewSettings.relativeEyePoint = true;

	// The new eyepoint position has been calculated from
	// its pitch, yaw and distance values.
	// To guarantee that the line of sight is not parallel to
	// the up-vector, the latter is recalculated.
	double const radUpYaw = ( radEyeYaw < pi ) ? radEyeYaw + pi : radEyeYaw - pi;
	double const radUpPitch = (pi / 2) - radEyePitch;

#ifdef _WINDOWS	
	double const sinUpYaw = ::sin(radUpYaw);
	double const cosUpYaw = ::cos(radUpYaw);
	double const sinUpPitch = ::sin(radUpPitch);
	double const cosUpPitch = ::cos(radUpPitch);
#else
	double const sinUpYaw = std::sin(radUpYaw);
	double const cosUpYaw = std::cos(radUpYaw);
	double const sinUpPitch = std::sin(radUpPitch);
	double const cosUpPitch = std::cos(radUpPitch);
#endif

	double upX = sinUpYaw * cosUpPitch;
	double upY = sinUpPitch;
	double upZ = cosUpYaw * cosUpPitch;

	visualisation->viewSettings.up.set(0, upX);
	visualisation->viewSettings.up.set(1, upY);
	visualisation->viewSettings.up.set(2, upZ);

	if (automaticRefresh)
	  update();
      };
  };

  void SIG_VisualisationWidget::initFloatingTextWidgets()
  {
    if (visualisation)
      {
	// clear() with autoDelete was the free for the previous labels; without
	// it they would survive as children of this widget and stay on screen.
	qDeleteAll( floatingTextWidgets );
	floatingTextWidgets.clear();
	floatingTextWidgets.resize( visualisation->floatingTexts.size() );

	for (int i=0; i<floatingTextWidgets.size(); i++)
	  {
	    SIG_FloatingTextLabel *newLabel = new SIG_FloatingTextLabel( this );
	    // Qt 2's QVector::insert overwrote slot i; QList::insert SHIFTS.
	    floatingTextWidgets[ i ] = newLabel;
	    newLabel->raise();
	  };
      };
  };

  void SIG_VisualisationWidget::mousePressEvent( QMouseEvent *event )
  {
    mouseXPos = event->x();
    mouseYPos = event->y();
  };

  void SIG_VisualisationWidget::mouseMoveEvent( QMouseEvent *event )
  {
    int deltaX = static_cast<int>( (event->x() - mouseXPos) * mouseSensity );
    int deltaY = static_cast<int>( (event->y() - mouseYPos) * mouseSensity );

    if (event->buttons() & Qt::LeftButton)
      emit signalMouseRotation( deltaX, deltaY );

    if (event->buttons() & Qt::RightButton)
      emit signalMouseZoom( deltaY );

    mouseXPos = event->x();
    mouseYPos = event->y();
  };

  void SIG_VisualisationWidget::initializeGL()
  {
    glClearColor( 1, 1, 1, 1 );
  };


  void SIG_VisualisationWidget::resizeGL(int width, int height)
  {
    GLsizei wwidth = static_cast<GLsizei>(width);
    GLsizei hheight = static_cast<GLsizei>(height);

    glViewport( 0, 0, wwidth, hheight );

    if (visualisation)
    {
      visualisation->viewSettings.aspectRatio = static_cast<double>(width) / static_cast<double>(height);
      visualisation->updateAspectRatio();
    };
  };

  void SIG_VisualisationWidget::paintGL()
    {
      if (visualisation)
	{
	  visualisation->visualize();

	  if (showAncorPointsState == 2)
	    {
	      for (int i=0; i<floatingTextWidgets.size(); i++)
		{
		  SIGEL_Visualisation::SIG_FloatingText *floatingText = visualisation->floatingTexts[i];
		  if (floatingText->rendered)
		    {
		      SIG_FloatingTextLabel *floatingTextLabel = floatingTextWidgets[i];

		      floatingTextLabel->setText( floatingText->textLabel );
		      QSize actSize = floatingTextLabel->sizeHint();
		      if (actSize.width() > floatingTextsSize.width())
			floatingTextsSize = actSize;

		      floatingTextLabel->move( floatingText->xPos,
					       this->height() - floatingText->yPos );
		    };
		};

	      for (int j=0; j<floatingTextWidgets.size(); j++)
		{
		  SIGEL_Visualisation::SIG_FloatingText *floatingText = visualisation->floatingTexts[j];
		  SIG_FloatingTextLabel *floatingTextLabel = floatingTextWidgets[j];
		  if (floatingText->rendered)
		    {
		      floatingTextLabel->resize( floatingTextsSize );
		      floatingTextLabel->show();
		    }
		  else
		    floatingTextLabel->hide();
		};
	    };
	}
      else
	glClear(GL_COLOR_BUFFER_BIT);
    };
//}
