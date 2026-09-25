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
#include "SIGEL_Visualisation/SIG_ViewSettings.h"

namespace SIGEL_Visualisation
{

  SIG_ViewSettings::SIG_ViewSettings()
    : eyePoint(0,1,1),
      lookPoint(0,0,0),
      up(0,1,0),
      relativeEyePoint(false),
      renderMode(wireFrame),
      aspectRatio(1)
  { };

  DL_vector SIG_ViewSettings::getAbsoluteEyePoint() const
  {
    if (!relativeEyePoint)
      return DL_vector( eyePoint.x, eyePoint.y, eyePoint.z );

    return DL_vector( lookPoint.x + eyePoint.x,
		      lookPoint.y + eyePoint.y,
		      lookPoint.z + eyePoint.z );
  };

}
