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
#include "SIGEL_Visualisation/SIG_Renderer.h"
#include "SIGEL_Tools/SIG_TypeConverter.h"

using namespace SIGEL_Tools;

namespace SIGEL_Visualisation
{

  SIG_Renderer::SIG_Renderer(int noOfObjects, int noOfFloatingTexts)
    : sceneObjects(noOfObjects),
      floatingTexts(noOfFloatingTexts)
  {
    this->noOfObjects = static_cast<GLuint>(noOfObjects); // ToDo: throw bad number error

    glListBase(0);
    displayListsOffset = glGenLists( this->noOfObjects ); // ToDo: throw allocation error
  };

  SIG_Renderer::~SIG_Renderer()
  {
    // In 1.3, setAutoDelete(true) on both vectors was their ONLY free --
    // ~QVector did it. Written out here.
    qDeleteAll( sceneObjects );
    sceneObjects.clear();
    qDeleteAll( floatingTexts );
    floatingTexts.clear();

    glDeleteLists( displayListsOffset, static_cast<GLsizei>(noOfObjects) );
  };

  void SIG_Renderer::renderSceneObjects( GLuint first, GLuint end )
  {
    glMatrixMode(GL_MODELVIEW);
    for (GLuint i=first; i < end; i++)
	if (sceneObjects[i]->getVisible())
	  {
	    sceneObjects[i]->applyColor();
	    glPushMatrix();
	    sceneObjects[i]->applyTransformation();
	    glCallList( displayListsOffset + i );
	    glPopMatrix();
	  };
  };

  QString SIG_Renderer::exportSceneObjectsToPovray()
  {
    QString resultString;
    QTextStream resultStream( &resultString, QIODevice::WriteOnly );

    for (int i=0; i < noOfObjects; i++)
      if (sceneObjects[ i ]->getVisible())
	{
	  NEWMAT::Matrix rotation = (   SIG_TypeConverter::sigelToPovray()
	                              * SIG_TypeConverter::toMatrix( sceneObjects[ i ]->getRotation() ) ).t();
	  NEWMAT::ColumnVector position =   SIG_TypeConverter::sigelToPovray()
	                                  * SIG_TypeConverter::toColumnVector( sceneObjects[ i ]->getPosition() );

	  resultStream << "object {\n"
		       << "  "
		       << sceneObjects[ i ]->name
		       << "\n"
		       << "  matrix <";
	  for (int i=1; i<=3; i++)
	    resultStream << " " << rotation( 1, i ) << ",";
	  resultStream << "\n"
		       << "          ";
	  for (int j=1; j<=3; j++)
	    resultStream << " " << rotation( 2, j ) << ",";
	  resultStream << "\n"
		       << "          ";
	  for (int k=1; k<=3; k++)
	    resultStream << " " << rotation( 3, k ) << ",";
	  resultStream << "\n"
		       << "           0 0 0 >\n"
		       << "  translate "
		       << vectorToPovray( position )
		       << "\n"
		       << "}\n"
		       << "\n";
	};

    return resultString;
  };

  QString SIG_Renderer::vectorToPovray( NEWMAT::ColumnVector input )
  {
    QString resultString;

    QTextStream resultStream( &resultString, QIODevice::WriteOnly );
    resultStream.setRealNumberPrecision( 5 );

    resultStream << "<" << input( 1 ) << ", " << input( 2 ) << ", " << input( 3 ) << ">";

    return resultString;
  };


}
