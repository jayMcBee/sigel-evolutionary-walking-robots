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
#ifndef SIGEL_GP_SIG_GPPVMDYNAMICCLIENTSERVER_H
#define SIGEL_GP_SIG_GPPVMDYNAMICCLIENTSERVER_H

#include "SIGEL_GP/SIG_GPFitnessTrainer.h"

#include <QList>

namespace SIGEL_GP
{

	/**
	 * The server that the dynamic PVM clients of an evolution connect to.
	 */
	class SIG_GPPVMDynamicClientServer
	{
	public:
		SIG_GPPVMDynamicClientServer();

		/**
		 * The body of the server thread. It runs until the program ends.
		 * It registers each client that submits its hostname with 'trainer'.
		 */
		void run(SIG_GPFitnessTrainer *trainer);

	private:
		enum
		{
			kSigelMasterRegPort = 6789
		};

		/**
		 * Opens the port that the clients connect to.
		 */
		void openPort();

		/**
		 * Waits up to 10 seconds. True when the server is to accept a client.
		 */
		bool waitForClient();

		/**
		 * Accepts one client and registers its host with 'trainer', or refuses it.
		 */
		void acceptClient(SIG_GPFitnessTrainer *trainer);

		/**
		 * The socket that listens on the port. Only the server thread uses it.
		 */
		int listenSocket;

		/**
		 * The sockets of the registered clients. Only the server thread uses them.
		 */
		QList<int> clientSockets;
	};

}

#endif // SIGEL_GP_SIG_GPPVMDYNAMICCLIENTSERVER_H
