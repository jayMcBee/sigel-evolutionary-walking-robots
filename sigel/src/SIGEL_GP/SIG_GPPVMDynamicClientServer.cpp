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
#include "SIGEL_GP/SIG_GPPVMDynamicClientServer.h"

#include <QList>
#include <QString>

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

SIGEL_GP::SIG_GPPVMDynamicClientServer::SIG_GPPVMDynamicClientServer()
	: allDisconnected( false ),
	  disconnectClients( false ),
	  serverIsUp( false ),
	  listenSocket( -1 )
{
	pthread_mutex_init(&disconnectMutex, nullptr);
	pthread_cond_init(&cond, nullptr);
}

void SIGEL_GP::SIG_GPPVMDynamicClientServer::run(SIG_GPFitnessTrainer *trainer)
{
	// init some variables
	serverIsUp = true;

	openPort();

	// the (almost) endless server loop
	while ( true )
	{
		if (waitForClient())
		{
			acceptClient(trainer);
		}

		disconnectClientsOnRequest();
	}
}

void SIGEL_GP::SIG_GPPVMDynamicClientServer::openPort()
{
	struct sockaddr_in  sad;
	struct protoent *tcpProtocol;

	// Make a socket to listen to our clients;
	// init sockaddr struct: using Internet family, port kSigelMasterRegPort
	memset(&sad, 0, sizeof(struct sockaddr_in));
	sad.sin_family = AF_INET;
	sad.sin_port = htons(kSigelMasterRegPort);
	sad.sin_addr.s_addr = INADDR_ANY;

	// map TCP protocol number
	if ((tcpProtocol = getprotobyname("tcp")) == nullptr)
	{
		fprintf(stderr, "ERR:   Can't map 'tcp' to a protocol number\n");
		exit(1);
	}

	// finally create the socket
	listenSocket = socket(PF_INET, SOCK_STREAM, tcpProtocol->p_proto);
	if (listenSocket < 0)
	{
		fprintf(stderr, "ERR:   Can't create socket\n");
		exit(1);
	}

	// let's bind local address & socket
	if ( bind(listenSocket, reinterpret_cast<struct sockaddr *>(&sad), sizeof(sad)) < 0 )
	{
		fprintf(stderr, "ERR:   Bind reported an error\n");
		exit(1);
	}

	// build the queue for incoming requests
	if ( listen(listenSocket, 32) < 0 )
	{
		fprintf(stderr, "ERR:   Listen failed\n");
		exit(1);
	}

	// (bounded) waiting for requests..
	fprintf(stderr, "Server is awaiting requests from dynamic clients on port %d..\n\n", kSigelMasterRegPort);
}

bool SIGEL_GP::SIG_GPPVMDynamicClientServer::waitForClient()
{
	// to use select() we need to build a fs_set first
	fd_set listenSet;
	FD_ZERO(&listenSet);
	FD_SET(listenSocket, &listenSet);

	// accept() is blocking, but we want to wait in 10 sec. chunks;
	// this allows the main thread to adjust it's active-pvm-host list based on
	// our freshly registered clients. Also check all 10 seconds if the main
	// thread wants us to cancel all connections.
	struct timeval timeOut;
	timeOut.tv_sec  = 10;
	timeOut.tv_usec = 0;

	// pselect returns zero when timeout occurs..
	select(listenSocket+1, &listenSet, nullptr, nullptr, &timeOut);

	return FD_ISSET(listenSocket, &listenSet);
}

void SIGEL_GP::SIG_GPPVMDynamicClientServer::acceptClient(SIG_GPFitnessTrainer *trainer)
{
	int sdRecv;
	char clientName[256];
	QString client;

	sdRecv = accept(listenSocket, nullptr, nullptr);

	if (sdRecv < 0)
	{
		fprintf(stderr, "ERR:   accept() failed\n");
		exit(1);
	}

	// a client that sends nothing must not block this thread
	struct timeval receiveTimeOut;
	receiveTimeOut.tv_sec  = 10;
	receiveTimeOut.tv_usec = 0;

	if (setsockopt(sdRecv, SOL_SOCKET, SO_RCVTIMEO, &receiveTimeOut, sizeof(receiveTimeOut)) < 0)
	{
		fprintf(stderr, "ERR:   setsockopt() failed for a dynamic client; its connection is closed\n");
		close(sdRecv);
		return;
	}

	const ssize_t receivedBytes = recv(sdRecv, clientName, sizeof(clientName), 0);

	if (receivedBytes <= 0)
	{
		fprintf(stderr, "ERR:   a dynamic client sent no host name; it is not registered\n");
		close(sdRecv);
		return;
	}

	if (memchr(clientName, '\0', receivedBytes) == nullptr)
	{
		fprintf(stderr, "ERR:   a dynamic client sent a host name with no end; its connection is closed\n");
		close(sdRecv);
		return;
	}

	// store socket for later disconnect
	clientSockets.resize( clientSockets.count()+1 );
	clientSockets[clientSockets.count()-1] = sdRecv;

	// remember client locally for later disconnect
	client = clientName;

	// tell the fitnesstrainer there's a fresh host
	trainer->addDynHost(client);
	fprintf(stderr, "\t(Servertask registered dyn. client \"%s\")\n", clientName);
}

void SIGEL_GP::SIG_GPPVMDynamicClientServer::disconnectClientsOnRequest()
{
	// check if computation is finished and clients need to be disconnected
	pthread_mutex_lock( &disconnectMutex );

	if (disconnectClients)
	{
		// iterate through list of connected sockets and cut connection;
		// be sure all clients have been removed from the pvmHost list !
		const int disconnectMessage = kSuicidalRequest;

		for (int i=0; i<clientSockets.count(); i++)
		{
			send( clientSockets[i], &disconnectMessage, sizeof(disconnectMessage), 0);
			close(clientSockets[i]);
		}
		fprintf(stderr, "\t(Servertask disconnected %d clients)\n", static_cast< int >(clientSockets.count()));
		clientSockets.resize(0);

		// tell main thread to continue !
		disconnectClients = false;
		allDisconnected = true;

		pthread_cond_broadcast(&cond);
	}

	pthread_mutex_unlock( &disconnectMutex );
}

void SIGEL_GP::SIG_GPPVMDynamicClientServer::releaseAllClients(SIG_GPFitnessTrainer *trainer)
{
	fprintf(stderr, " Releasing Dynamic SIGEL-Clients:\n");

	// make the main thread running exclusively
	pthread_mutex_lock( &disconnectMutex );
	fprintf(stderr, "\t- Flushing all dynamic clients from PVM-Hosts list\n");
	trainer->flushAllDynHosts();

	fprintf(stderr, "\t- Asking server to disconnect the clients\n");
	disconnectClients = true;
	allDisconnected = false;

	// let's wait for server thread
	while ( ! allDisconnected )
	{
		pthread_cond_wait(&cond, &disconnectMutex);
	}
	pthread_mutex_unlock( &disconnectMutex );

	fprintf(stderr, "\n");
}

bool SIGEL_GP::SIG_GPPVMDynamicClientServer::isRunning() const
{
	return serverIsUp;
}
