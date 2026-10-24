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
#include "SIGEL_Environment/SIG_Terrain.h"

#include "SIGEL_Tools/SIG_Exception.h"
#include "SIGEL_Tools/SIG_IO.h"
#include "fparser.h"

#include <QByteArray>

#include <fstream>
#include <string>

SIGEL_Environment::SIG_Terrain::SIG_Terrain()
	: dimensionX( 0 ),
	  dimensionZ( 0 )
{
}

QList<SIGEL_Environment::SIG_TerrainPreset> const &SIGEL_Environment::SIG_Terrain::presets()
{
	static QList<SIG_TerrainPreset> const list =
	{
		{ "Valley", "0.15*abs(x-cx)" },
		{ "Hill", "4*exp(-((x-cx)^2+(z-cz)^2)/100)" },
		{ "Washboard", "0.3*sin(x/1.5)" },
		{ "Crater", "2.5*exp(-((sqrt((x-cx)^2+(z-cz)^2)-12)^2)/10)" },
		{ "Gravel", "0.15*abs(sin(x*12.9898+z*78.233)*43758.5453%1)" },
		{ "Ripples", "-sin(sqrt(x^2+z^2))" },
		{ "Rice terraces", "0.5*floor(8*exp(0-((x-cx)^2+(z-cz)^2)/250))" },
		{ "Amphitheatre", "0.3*floor(sqrt((x-cx)^2+(z-cz)^2)/3)" },
		{ "Speed bumps", "0.3*(1+cos(x*0.8))^3/8" },
		{ "Volcano", "4*exp(0-((x-cx)^2+(z-cz)^2)/150)-2.5*exp(0-((x-cx)^2+(z-cz)^2)/12)" },
	};
	return list;
}

QString SIGEL_Environment::SIG_Terrain::presetFunction( SIG_TerrainPreset const &preset, int dimensionX, int dimensionZ )
{
	// The grid points are 1 apart and start at 0, so the middle of X points is (X - 1) / 2.
	QString function = preset.function;
	function.replace( "cx", QString::number( ( dimensionX - 1 ) / 2.0 ) );
	function.replace( "cz", QString::number( ( dimensionZ - 1 ) / 2.0 ) );
	return function;
}

bool SIGEL_Environment::SIG_Terrain::setFromFunction( const QString &function, int newDimensionX, int newDimensionZ )
{
	FunctionParser fp;
	if ( fp.Parse( function.toUtf8().constData(), "xz" ) != -1 )
	{
		SIGEL_Tools::SIG_IO::cerr << "Warning: the specified terrain function could not be parsed. It must depend on x and z, and contain only valid expressions." << Qt::endl;
		return false;
	}

	QList<double> newHeights;
	double vals[2];

	for ( int z = 0; z < newDimensionZ; ++z )
		for ( int x = 0; x < newDimensionX; ++x )
		{
			vals[0] = x;
			vals[1] = z;
			newHeights.append( fp.Eval( vals ) );
		}

	dimensionX = newDimensionX;
	dimensionZ = newDimensionZ;
	heights = newHeights;
	return true;
}

bool SIGEL_Environment::SIG_Terrain::setFromPictureFile( const QString &pictureFile )
{
	std::ifstream pgm( pictureFile.toUtf8().constData() );
	if ( !pgm )
	{
		SIGEL_Tools::SIG_IO::cerr << "Warning: the specified terrain file does not exist." << Qt::endl;
		return false;
	}

	int newDimensionX = 0;
	int newDimensionZ = 0;
	QList<double> newHeights;
	std::string s;
	double max = 0;
	int counter = 0;

	while ( !pgm.eof() )
	{
		pgm >> s; // read file content till white space

		if ( s.find( "#", 0 ) != std::string::npos )
		{
			pgm.ignore( 255, '\n' );
			continue;
		}

		switch ( counter )
		{
		case 0: // Magic Key
			if ( s.compare( "P2" ) != 0 )
			{
				SIGEL_Tools::SIG_IO::cerr << "Warning: the specified picture file is not in PGM format." << Qt::endl;
				return false;
			}
			++counter;
			break;
		case 1: // x-DIM
			newDimensionX = QString( s.c_str() ).toInt();
			++counter;
			break;
		case 2: // y-DIM
			newDimensionZ = QString( s.c_str() ).toInt();
			++counter;
			break;
		case 3: // max. grey-value
			max = QString( s.c_str() ).toDouble();
			++counter;
			break;
		default:
			newHeights.append( QString( s.c_str() ).toDouble() / max * 5 );
		}
	}

	dimensionX = newDimensionX;
	dimensionZ = newDimensionZ;
	heights = newHeights;
	return true;
}

void SIGEL_Environment::SIG_Terrain::setFlat( int newDimensionX, int newDimensionZ )
{
	dimensionX = newDimensionX;
	dimensionZ = newDimensionZ;
	heights = QList<double>( newDimensionX * newDimensionZ, 0.0 );
}

int SIGEL_Environment::SIG_Terrain::getDimensionX() const
{
	return dimensionX;
}

int SIGEL_Environment::SIG_Terrain::getDimensionZ() const
{
	return dimensionZ;
}

double SIGEL_Environment::SIG_Terrain::getHeight( int x, int z ) const
{
	return heights[ z * dimensionX + x ];
}

bool SIGEL_Environment::SIG_Terrain::isFlat() const
{
	return heights.count( 0.0 ) == heights.size();
}

void SIGEL_Environment::SIG_Terrain::writeToFile( QTextStream &file ) const
{
	file << dimensionX << " " << dimensionZ << "\n";

	// 17 significant digits are enough to read back exactly the same double.
	for ( int z = 0; z < dimensionZ; ++z )
	{
		for ( int x = 0; x < dimensionX; ++x )
			file << QString::number( getHeight( x, z ), 'g', 17 ) << " ";

		file << "\n";
	}

	// Without it, a text cut inside the last height would read as a shorter number.
	file << "TerrainComplete\n";
}

void SIGEL_Environment::SIG_Terrain::readFromFile( QTextStream &file )
{
	int newDimensionX = 0;
	int newDimensionZ = 0;
	file >> newDimensionX >> newDimensionZ;

	if ( file.status() != QTextStream::Ok || newDimensionX < 0 || newDimensionZ < 0 )
	{
		throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__, "The terrain has no valid dimensions." );
	}

	QList<double> newHeights;

	for ( qsizetype i = 0; i < qsizetype( newDimensionX ) * newDimensionZ; ++i )
	{
		double height = 0;
		file >> height;

		if ( file.status() != QTextStream::Ok )
		{
			throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__, "The terrain ends before its last height." );
		}

		newHeights.append( height );
	}

	QString marker;
	file >> marker;

	if ( marker != "TerrainComplete" )
	{
		throw SIGEL_Tools::SIG_Exception( __FILE__, __LINE__, "The terrain ends before its last height." );
	}

	dimensionX = newDimensionX;
	dimensionZ = newDimensionZ;
	heights = newHeights;
}
