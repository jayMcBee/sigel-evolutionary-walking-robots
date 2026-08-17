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
#ifndef SIGEL_GP_SIG_GPEXCEPTION_H
#define SIGEL_GP_SIG_GPEXCEPTION_H

#include "SIGEL_Tools/SIG_Exception.h"
using namespace SIGEL_Tools;	

namespace SIGEL_GP
{

/**
* This class contains the SIG_GPExceptions. It is needed to secure the correct run of the SIGEL_GP_Package.
*
*/

	class SIG_GPException : public SIGEL_Tools::SIG_Exception{
		
		public: 
			SIG_GPException (QString file, int line, QString msg);		
	};

	
	class SIG_GPPolPosNotValidException : public SIG_Exception{
		public: 
			SIG_GPPolPosNotValidException (QString file,
                                        	      int line,
                                       		      QString msg);		
	};
	
	class SIG_GPIndIsTooOldException : public SIG_Exception{
		public: 
			SIG_GPIndIsTooOldException (QString file,
                                        	      int line,
                                       		      QString msg);					
	};


}
#endif //  SIGEL_GP_SIG_GPEXCEPTION_H


