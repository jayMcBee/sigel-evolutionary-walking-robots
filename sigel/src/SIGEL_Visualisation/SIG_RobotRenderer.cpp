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
#include <QIODevice>
#include "SIGEL_Visualisation/SIG_RobotRenderer.h"
#include "SIGEL_Tools/SIG_TypeConverter.h"

using namespace SIGEL_Tools;

namespace SIGEL_Visualisation
{

  SIG_RobotRenderer::SIG_RobotRenderer(SIGEL_Robot::SIG_Robot const &robot)
    : SIG_Renderer( robot.getLinks().size() + robot.getNrOfPoints(), robot.getNrOfPoints() ),
      robot(robot)
  {
    for ( SIGEL_Robot::SIG_Link *linkPtr : robot.getLinks() )
      {
	SIGEL_Robot::SIG_Link &actLink = *linkPtr;

	int number = actLink.getNumber();
	QString name = actLink.getName();

	SIG_VisualSceneObject *newSceneObject = new SIG_VisualSceneObject(number, name);
	newSceneObject->setColor( actLink.getMaterial()->getColour() );

	delete sceneObjects[ number ];
	sceneObjects[ number ] = newSceneObject;

      };

    buildDisplayLists();

    setPointsVisible( false );
  };

  SIG_RobotRenderer::~SIG_RobotRenderer()
  { };

  void SIG_RobotRenderer::buildDisplayLists()
  {
    int nextPointIndex = robot.getLinks().size();
    int nextFloatingIndex = 0;

    for ( SIGEL_Robot::SIG_Link *linkPtr : robot.getLinks() )
      {
	SIGEL_Robot::SIG_Link &actLink = *linkPtr;

	int linkNumber = actLink.getNumber();
	GLuint linkIndex = static_cast<GLuint>(linkNumber);
	GLuint actListIndex = linkIndex + displayListsOffset;

	glNewList(actListIndex, GL_COMPILE);

	SIGEL_Robot::SIG_Geometry const *actGeometry = actLink.getGeometry();
	SIGEL_Robot::SIG_GeometryIterator actGeoIter(actGeometry);

	glFrontFace( GL_CCW );

	while (actGeoIter.valid())
	  {
	    SIGEL_Robot::SIG_Polygon const &actPolygon = actGeoIter.current();

	    DL_vector vertex0 = actPolygon.getVertex( 0 );
	    DL_vector vertex1 = actPolygon.getVertex( 1 );
	    DL_vector vertex2 = actPolygon.getVertex( 2 );

	    DL_vector u = vertex0;
	    u.minusis( &vertex1 );

	    DL_vector v = vertex2;
	    v.minusis( &vertex1 );

	    DL_vector polygonNormal;

	    v.crossprod( &u, &polygonNormal );

	    glNormal3f( static_cast<GLfloat>( polygonNormal.get(0) ),
			static_cast<GLfloat>( polygonNormal.get(1) ),
			static_cast<GLfloat>( polygonNormal.get(2) ) );

	    glBegin(GL_POLYGON);
	    for (int i=0; i<actPolygon.getNumVertices(); i++)
	      {
		DL_vector actVertex = actPolygon.getVertex(i);
		glVertex3d( static_cast<GLdouble>(actVertex.get(0)),
			    static_cast<GLdouble>(actVertex.get(1)),
			    static_cast<GLdouble>(actVertex.get(2)) );
	      };
	    glEnd();

	    actGeoIter.next();
	  };

	glEndList();

	for ( const SIGEL_Robot::SIG_Link::NamedPoint &pointEntry : actLink.getPoints() )
	  {
	    GLuint pointIndex = static_cast<GLuint>( nextPointIndex );

	    GLuint actListIndex = pointIndex + displayListsOffset;

	    DL_vector pointPosition = *pointEntry.value;

	    glNewList( actListIndex, GL_COMPILE );
	    glPointSize( 5 );
	    glBegin( GL_POINTS );
	    glVertex3d( static_cast<GLdouble>(pointPosition.get(0)),
			static_cast<GLdouble>(pointPosition.get(1)),
			static_cast<GLdouble>(pointPosition.get(2)) );
	    glEnd();
	    glEndList();

	    SIG_VisualSceneObject *newPointSceneObject = new SIG_VisualSceneObject( linkNumber,
										    pointEntry.name );

	    SIG_FloatingText *newFloatingText = new SIG_FloatingText( 0, 0, pointEntry.name );

	    newPointSceneObject->setFloatingText( newFloatingText );

	    DL_vector linkColor = actLink.getMaterial()->getColour();

	    for (int i=0; i<3; i++)
	      {
		double newValue = linkColor.get(i) - 0.3;
		newValue = ( newValue < 0 ) ? 0 : newValue;
		linkColor.set( i, newValue );
	      };

	    newPointSceneObject->setColor( linkColor );

	    delete sceneObjects[ nextPointIndex ];
	    sceneObjects[ nextPointIndex ] = newPointSceneObject;

	    delete floatingTexts[ nextFloatingIndex ];
	    floatingTexts[ nextFloatingIndex ] = newFloatingText;

	    nextPointIndex++;
	    nextFloatingIndex++;
	  };

      };

  };

  void SIG_RobotRenderer::render()
  {
    renderLinks();
    renderPoints();
  };

  void SIG_RobotRenderer::renderLinks()
  {
    renderSceneObjects( 0, static_cast<GLuint>(robot.getLinks().size()) );
  };

  void SIG_RobotRenderer::renderPoints()
  {
    renderSceneObjects( static_cast<GLuint>(robot.getLinks().size()), noOfObjects );

    for (GLuint i=static_cast<GLuint>(robot.getLinks().size()); i<noOfObjects; i++)
      {
	QList< GLfloat > buffer( 3 );
	buffer.fill( 0 );

	glFeedbackBuffer( 3, GL_2D, buffer.data() );
	glRenderMode(GL_FEEDBACK);

	glPushMatrix();
	sceneObjects[i]->applyTransformation();

	glCallList( displayListsOffset + i );
	glPopMatrix();

	glRenderMode(GL_RENDER);

	if ( buffer[0] == GL_POINT_TOKEN )
	  {
	    sceneObjects[i]->getFloatingText()->rendered = true;
	    sceneObjects[i]->getFloatingText()->xPos = static_cast<int>( buffer[1] );
	    sceneObjects[i]->getFloatingText()->yPos = static_cast<int>( buffer[2] );
	  }
	else
	  sceneObjects[i]->getFloatingText()->rendered = false;
      };
  };

  QString SIG_RobotRenderer::exportToPovray()
  {
    return exportSceneObjectsToPovray();
  };

  QString SIG_RobotRenderer::createPovrayDeclarations()
  {
    QString declarationsString;

    QTextStream stream( &declarationsString, QIODevice::WriteOnly );

    for ( SIGEL_Robot::SIG_Material *materialPtr : robot.getMaterials() )
      {
	SIGEL_Robot::SIG_Material &actMaterial = *materialPtr;

	stream << "#declare "
	       << actMaterial.getName()
	       << " = texture {\n"
	       << "  pigment { rgb "
	       << SIG_Renderer::vectorToPovray( SIG_TypeConverter::toColumnVector( actMaterial.getColour() ) )
	       << " }\n"
	       << "  finish { ambient rgb "
	       << SIG_Renderer::vectorToPovray( SIG_TypeConverter::toColumnVector( actMaterial.getColour() ) )
	       << "\n"
	       << "           diffuse 1 }\n"
	       << "}\n"
	       << "\n";

      };

    QString pointDataString;
    QTextStream pointDataStream( &pointDataString, QIODevice::WriteOnly );

    for ( SIGEL_Robot::SIG_Link *linkPtr : robot.getLinks() )
      {
	SIGEL_Robot::SIG_Link &actLink = *linkPtr;

	int linkNumber = actLink.getNumber();

	QString polygonDataString;
	QTextStream polygonDataStream( &polygonDataString, QIODevice::WriteOnly );

	stream << "#declare "
	       << actLink.getName()
	       << " = mesh {\n";

	SIGEL_Robot::SIG_Geometry const *actGeometry = actLink.getGeometry();
	SIGEL_Robot::SIG_GeometryIterator actGeoIter(actGeometry);

	while (actGeoIter.valid())
	  {
	    SIGEL_Robot::SIG_Polygon const &actPolygon = actGeoIter.current();

	    NEWMAT::ColumnVector firstVertex = SIG_TypeConverter::toColumnVector( actPolygon.getVertex( 0 ) );

	    NEWMAT::ColumnVector secondVertex = SIG_TypeConverter::toColumnVector( actPolygon.getVertex( 1 ) );

	    for ( int i=2; i < actPolygon.getNumVertices(); i++ )
	      {
		NEWMAT::ColumnVector thirdVertex = SIG_TypeConverter::toColumnVector( actPolygon.getVertex( i ) );

		polygonDataStream << "  triangle {\n";

		polygonDataStream << "    "
				  << SIG_Renderer::vectorToPovray( firstVertex )
				  << ", "
				  << SIG_Renderer::vectorToPovray( secondVertex )
				  << ", "
				  << SIG_Renderer::vectorToPovray( thirdVertex )
				  << "\n"
				  << "  }\n";

		secondVertex = thirdVertex;
	      };

	    actGeoIter.next();
	  };

	for ( const SIGEL_Robot::SIG_Link::NamedPoint &pointEntry : actLink.getPoints() )
	  {
	    NEWMAT::ColumnVector pointPosition = SIG_TypeConverter::toColumnVector( *pointEntry.value );

	    NEWMAT::ColumnVector linkColor = SIG_TypeConverter::toColumnVector( actLink.getMaterial()->getColour() );

	    for (int i=1; i<=3; i++)
	      {
		double newValue = linkColor( i ) - 0.3;
		newValue = ( newValue < 0 ) ? 0 : newValue;
		linkColor( i ) = newValue;
	      };

	    pointDataStream << "#declare "
			    << pointEntry.name
			    << " = sphere {\n"
			    << "  "
			    << vectorToPovray( pointPosition )
			    << ", 0.05\n"
			    << "  texture {\n"
			    << "    "
			    << actLink.getMaterial()->getName()
			    << "\n"
			    << "    pigment { rgb "
			    << vectorToPovray( linkColor )
			    << " }\n"
			    << "  }\n"
			    << "}\n"
			    << "\n";

	  };

	stream << polygonDataString
	       << "\n"
	       << "  texture { "
	       << actLink.getMaterial()->getName()
	       << "  }\n"
	       << "}\n"
	       << "\n";
      };

    stream << pointDataString;

    return declarationsString;
  };

  void SIG_RobotRenderer::setPointsVisible( bool visible )
  {
    for (int i = robot.getLinks().size(); i < noOfObjects; i++ )
      sceneObjects[i]->setVisible( visible );
  };

}
