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
#include "MT_GPSystem/MT_Operand.h"

MT_Operand::MT_Operand()
{
	OPType=1;
	VariableName=0;
	Data =-1;
}

MT_Operand::MT_Operand(MT_Operand *Original)
{
	OPType = Original->OPType;
	VariableName= Original->VariableName;
	Data= Original->Data;
}

MT_Operand::MT_Operand(int Op, double Da, int na)
{
	OPType = Op;

	if (OPType == 1)  // if Operand is a variable 
	{
		VariableName = na;
		Data = -1;
	}
	else			// or Operand is a constant
	{ 
		Data= Da;
		VariableName = -1;
	}
}
