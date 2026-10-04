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
#include "SIGEL_RobotCheck/SIG_RobotIssue.h"

bool SIGEL_RobotCheck::SIG_RobotIssue::operator<( const SIG_RobotIssue &other ) const
{
  if ( kind != other.kind )
    return kind < other.kind;

  return check < other.check;
}

SIGEL_RobotCheck::SIG_RobotIssue SIGEL_RobotCheck::SIG_RobotIssue::cannotBeSimulated( const QString &reason )
{
  return SIG_RobotIssue{ tCannotBeSimulated, tError, "Robot cannot be simulated", QString(), reason + " No program can be scored.",
                         "Correct the robot model or the setting that the analysis names." };
}
