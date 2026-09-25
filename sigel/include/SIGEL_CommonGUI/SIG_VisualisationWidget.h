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
#ifndef SIGEL_COMMONGUI_SIG_VISUALISATIONWIDGET_H
#define SIGEL_COMMONGUI_SIG_VISUALISATIONWIDGET_H

#include <QList>
#include <QOpenGLWidget>
#include <QLabel>

#include "SIGEL_Visualisation/SIG_Visualisation.h"
#include "SIGEL_CommonGUI/SIG_FloatingTextLabel.h"

using namespace SIGEL_CommonGUI;

/**
 * This namespace contains GUI-classes that are used both in the
 * Master's GUI and the Slave's GUI.
 */
//namespace SIGEL_CommonGUI		// The visual c++ compiler gets in trouble with the
//{										// corresponding moc file if this namespace is used
											// and moc ignore preprocessor statements.

  /**
   * This is the superclass of all widget-classes that should
   * be used to integrate the OpenGL-Visualisation-classes
   * into the GUI.
   *
   * It inherits from QOpenGLWidget and extends it with functionality
   * used in all subclasses, which finally are instantiated in the
   * GUI widgets that contain 3D views of a robot, an environment
   * or a simulation. This includes easily changing the eyepoint
   * (due to yaw and pitch angles and a distance value), lookpoint
   * and OpenGL related stuff involved in widget resizing for example.
   * To provide this functionality it need some basic features
   * that have all visualisation-classes in common. That's why
   * this class aggregates a pointer to the SIG_Visualisation superclass.
   */
  class SIG_VisualisationWidget : public QOpenGLWidget
    {
      Q_OBJECT

    public:

      /**
       * The constructor of the SIG_VisualisationWidget.
       *
       * Initializes the eyepoint and calculates @f$\pi@f$.
       * MouseSensity is set to 0\. 5 at the moment.
       *
       * @param parent Pointer to this widget's parent.
       * @param name This widget's name.
       * @param f The widget flag of this widget.
       */
      SIG_VisualisationWidget( QWidget *parent=0,
			       char const *name=0,
			       Qt::WindowFlags f = Qt::WindowFlags() );

      /**
       * The destructor of the SIG_VisualisationWidget.
       */
      ~SIG_VisualisationWidget();

    public slots:

      virtual void setRenderMode( const QString & string );

      virtual void setAmbientLighting( int newValue );

      virtual void setShowAncorPoints( int state );

      /**
       * This method sets the eyepoint's yaw angle.
       *
       * If automaticRefresh is true, the widget's contents are
       * refreshed (now according to the new eyepoint).
       *
       * @param yyaw The yaw angle is supplied in degrees between
       *             0 and 360.
       */
      virtual void setYaw( double yyaw );

      /**
       * This method sets the eyepoint's pitch angle.
       *
       * If automaticRefresh is true, the widget's contents are
       * refreshed (now according to the new eyepoint).
       *
       * @param yyaw The pitch angle is supplied in degrees between
       *             0 and 90.
       */
      virtual void setPitch( double ppitch );

      /**
       * This method sets the distance between eyepoint and lookpoint.
       *
       * If automaticRefresh is true, the widget's contents are
       * refreshed (now according to the new eyepoint).
       *
       * @param yyaw The new distance.
       */ 
      virtual void setDistance( double ddistance );

    signals:

      /**
       * This signal is emmited when the user has moved
       * the mouse.
       *
       * The supplied values are calculated by multiplying
       * the mouseSensity with the amount of pixels the
       * mouse was moved across the x- respectively y-axis
       * while the left mouse button was pressed.
       * This signal should be intercepted by the widget
       * containing this SIG_Visualisationwidget and effect
       * appropriate calling of the slots setYaw and setPitch.
       */
      void signalMouseRotation(int, int);

      /**
       * This signal is emmited when the user has moved the mouse.
       *
       * The supplied value is calculated by multiplying
       * the mouseSensity with the amount of pixels the mouse
       * was moved across the y-axis while the right mouse button
       * was pressed.
       * This signal should be intercepted by the widget
       * containing this SIG_Visualisationwidget and effect
       * appropriate calling of the slot setDistance.
       */
      void signalMouseZoom(int);

    protected:

      /**
       * Initaliazes some OpenGL stuff.
       *
       * This method is inherited from the QOpenGLWidget.
       * It is called automatically by the QT runtime system.
       * Because some initialization is already done in
       * the constructor of the contained SIG_Visualisation,
       * there's not much to do here. At the moment just the
       * glClearColor is set to white.
       */
      virtual void initializeGL();

      /**
       * Resizes the contents of this widget.
       *
       * This methid is inherited from the QOpenGLWidget.
       * It is called automatically by the QT runtime system.
       * The OpenGL viewport is resizes. If visualisation
       * points to a SIG_Visualisation object,
       * its aspect ratio is updated.
       *
       * @parameter width The widget's new width in pixels.
       * @parameter height The widget's new height in pixels.
       */
      virtual void resizeGL(int width, int height);

      /**
       * Refreshes the contents of this widget by initiating
       * the execution of appropriate OpenGL drawing commands.
       *
       * This methid is inherited from the QOpenGLWidget.
       * It is called automatically by the QT runtime system.
       * In some cases it is also called by other methods
       * (when changing the eyepoint while automaticRefresh
       * being true). This is only done indirectly by calling
       * the method update.
       */
      virtual void paintGL();

      /**
       * Calculates the actual eyepoint from the attributes
       * yaw, pitch and distance.
       *
       * If automaticRefresh is true, the widget's contents are
       * refreshed.
       */
      virtual void updateEyePoint();

      void initFloatingTextWidgets();

      /**
       * This method is inherited from the QWidget.
       *
       * When the user presses a mousebutton the actual
       * mouse position is stored in mouseXPos and mouseYPos.
       */
      virtual void mousePressEvent( QMouseEvent *event );

      /**
       * This method is inherited from the QWidget.
       *
       * When the user moves the mouse,
       * this method emits the appropriate signals.
       */
      virtual void mouseMoveEvent( QMouseEvent *event );

      /**
       * The pointer to the SIG_Visualisation object that is
       * finally responsible for the drawing of this widgets
       * contents.
       */
      SIGEL_Visualisation::SIG_Visualisation *visualisation;

      /**
       * The eyepoints actual yaw angle.
       */
      double yaw;

      /**
       * The eyepoints actual pitch angle.
       */
      double pitch;

      /**
       * The actual distance between the eyepoint and the lookpoint.
       */
      double distance;

      /**
       * The last X-position of the mouse.
       *
       * This is used to calculate the amount of pixels the mouse
       * was moved since the last mousePresseEvent respectively
       * mouseMoveEvent.
       */
      int mouseXPos;

      /**
       * The last Y-position of the mouse.
       *
       * This is used to calculate the amount of pixels the mouse
       * was moved since the last mousePresseEvent respectively
       * mouseMoveEvent.
       */
      int mouseYPos;

      /**
       * If true, changing of the eyepoint effects refreshing
       * this widget.
       */
      bool automaticRefresh;

      /**
       * The mouse sensity.
       *
       * A higher value increases the effect
       * of mouse movements.
       */
      double const mouseSensity;

      /**
       * The mathematical constant @f$\pi@f$.
       *
       * Its value is calculated in the constructor
       * as @f$arctan(1)\cdot 4@f$.
       */
      double const pi;

      QList< SIGEL_CommonGUI::SIG_FloatingTextLabel * > floatingTextWidgets;

      QSize floatingTextsSize;

      int showAncorPointsState;

    };

//}

#endif // SIGEL_COMMONGUI_SIG_VISUALISATIONWIDGET_H
