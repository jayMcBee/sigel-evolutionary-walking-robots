/*
  Copyright 2001 Christian Aue, Abdeladim Benkacem, Jens Busch,
                 Michael Gregorius, Andree Ross, Abdallah Salah Raiyan,
                 Daniel Sawitzki, Volker Strunk, Holger Tuerk,
                 Mihai-Christian Varcol, Jens Ziegler
  Copyright 2026 Jan Barnholt (SIGEL 2.0)

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
#include "SIGEL_Robot/SIG_VRMLUtils.h"
#include "SIGEL_Robot/SIG_VRMLFileError.h"

#include <CyberVRML97.h>
#include <QFile>
#include <cmath>

namespace SIGEL_Robot
{

	int SIG_VRMLUtils::lineOfUseAfterNodeEnd( QString const &vrml )
	{
		int line = 1;
		bool afterNodeEnd = false;
		qsizetype i = 0;

		// The text splits into tokens as in CyberVRML97's vrml.l.
		while (i < vrml.size())
		{
			QChar const c = vrml.at( i );
			if (c == '#')
			{
				i = indexAfterComment( vrml, i );
				continue;
			}

			if (c == '"')
			{
				qsizetype const end = indexAfterString( vrml, i );
				line += QStringView( vrml ).mid( i, end - i ).count( u'\n' );
				i = end;
				afterNodeEnd = false;
				continue;
			}

			if (isPartOfWord( c ))
			{
				qsizetype const end = indexAfterWord( vrml, i );
				if (afterNodeEnd && QStringView( vrml ).mid( i, end - i ) == u"USE")
					return line;

				i = end;
				afterNodeEnd = false;
				continue;
			}

			if (c == '\n')
				line++;
			else if (!c.isSpace())
				afterNodeEnd = (c == '}');

			i++;
		}
		return 0;
	}

	NEWMAT::Matrix SIG_VRMLUtils::vrmlRotationMatrix( QList< float > const &rotation )
	{
		NEWMAT::Matrix rotationMatrix(4,4);
		double const x = rotation[0];
		double const y = rotation[1];
		double const z = rotation[2];
		double const a = rotation[3];

		double const sinA = std::sin(a);
		double const cosA = std::cos(a);
		double const t = 1 - cosA;

		rotationMatrix = 0;
		rotationMatrix(1,1) = (t * x * x) + cosA;
		rotationMatrix(1,2) = (t * x * y) - (sinA * z);
		rotationMatrix(1,3) = (t * x * z) + (sinA * y);
		rotationMatrix(2,1) = (t * x * y) + (sinA * z);
		rotationMatrix(2,2) = (t * y * y) + cosA;
		rotationMatrix(2,3) = (t * y * z) - (sinA * x);
		rotationMatrix(3,1) = (t * x * z) - (sinA * y);
		rotationMatrix(3,2) = (t * y * z) + (sinA * x);
		rotationMatrix(3,3) = (t * z * z) + cosA;
		rotationMatrix(4,4) = 1;

		return rotationMatrix;
	}

	NEWMAT::Matrix SIG_VRMLUtils::transformNodeMatrix( TransformNode *transformNode )
	{
		QList< float > buffer(4);

		NEWMAT::Matrix identity(4,4);
		identity = 0;
		for (int i=1; i<=4; i++)
			identity(i,i) = 1;

		NEWMAT::Matrix center(4,4);
		center = identity;
		transformNode->getCenter( buffer.data() );
		for (int j=0; j<3; j++)
			center( j+1, 4 ) = buffer[j];

		NEWMAT::Matrix rotation(4,4);
		transformNode->getRotation( buffer.data() );
		rotation = vrmlRotationMatrix( buffer );

		NEWMAT::Matrix scale(4,4);
		scale = 0;
		scale(4,4) = 1;
		transformNode->getScale( buffer.data() );
		for (int k=0; k<3; k++)
			scale( k+1, k+1 ) = buffer[k];

		NEWMAT::Matrix scaleOrientation(4,4);
		transformNode->getScaleOrientation( buffer.data() );
		scaleOrientation = vrmlRotationMatrix( buffer );

		NEWMAT::Matrix translation(4,4);
		translation = identity;
		transformNode->getTranslation( buffer.data() );
		for (int l=0; l<3; l++)
			translation( l+1, 4 ) = buffer[l];

		return translation
			* center
			* rotation
			* scaleOrientation
			* scale
			* scaleOrientation.i()
			* center.i();
	}

	void SIG_VRMLUtils::loadScene( SceneGraph &scene, QString const &fileName )
	{
		QFile vrmlFile( fileName );
		if (!vrmlFile.open( QIODeviceBase::ReadOnly ))
		{
			QString const message = "cannot be read.";
			throw SIG_VRMLFileError( __FILE__, __LINE__, fileName, message );
		}

		int const useLine = lineOfUseAfterNodeEnd( QString::fromUtf8( vrmlFile.readAll() ) );
		if (useLine)
		{
			QString const message = "USE directly after \"}\" is not supported. Put a comma after the \"}\".";
			throw SIG_VRMLFileError( __FILE__, __LINE__, fileName, useLine, message );
		}

		QByteArray fileNameBytes = fileName.toUtf8();
		scene.load( fileNameBytes.data() );

		// CyberVRML97 keeps the nodes it parsed before an error, which would give a partial scene.
		if (!scene.isOK())
		{
			QString message = "syntax error.";
			QString const errorToken = QString::fromUtf8( scene.getErrorToken() );
			if (!errorToken.isEmpty())
				message = "syntax error at \"" + errorToken + "\".";

			int const errorLine = scene.getErrorLineNumber();
			throw SIG_VRMLFileError( __FILE__, __LINE__, fileName, errorLine, message );
		}
	}

	qsizetype SIG_VRMLUtils::indexAfterComment( QString const &vrml, qsizetype start )
	{
		qsizetype const end = vrml.indexOf( u'\n', start );
		if (end < 0)
			return vrml.size();

		return end;
	}

	qsizetype SIG_VRMLUtils::indexAfterString( QString const &vrml, qsizetype start )
	{
		qsizetype const end = vrml.indexOf( u'"', start + 1 );
		if (end < 0)
			return vrml.size();

		return end + 1;
	}

	qsizetype SIG_VRMLUtils::indexAfterWord( QString const &vrml, qsizetype start )
	{
		qsizetype end = start;
		while (end < vrml.size() && isPartOfWord( vrml.at( end ) ))
			end++;
		return end;
	}

	bool SIG_VRMLUtils::isPartOfWord( QChar c )
	{
		return c.isLetterOrNumber() || c == '_' || c == '-' || c == '+' || c == '.';
	}

}
