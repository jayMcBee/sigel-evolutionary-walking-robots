/*  <manage_dyn_slave.c>  --  Part of the SIGEL project
 *
 *  This little program has two purposes:
 *     1)  Register the local host.
 *     2)  Wait for a self-destruct command from the sigel_master
 *
 *  If the sigel master is launched in dynamic-evolve mode it listens to
 *  incoming connections on port 6789.
 *
 *  Once connected this program transmits the name of the local host,
 *  that's all. Thus the sigel master knows our local host has a sigel_slave
 *  binary (and supporting files) installed in /tmp/_SIGEL_EVOLUTION_TEMP
 *
 *  Make sure the latter is the case by launching the installSigelTmpEvolver
 *  script on the dedicated host.
 *
 *  When the computation is finished and the sigel_master application
 *  decides this host is no longer needed, it sends the self-destruct
 *  command to all temp. slaves, which in turn makes this slave remove
 *  the /tmp/_SIGEL_EVOLUTION_TEMP directory.
 *
 *  Author:  Jan Barnholt  <jan-b@uni.de>, <jan.barnholt@epost.de>
 */

#include <sys/types.h>
#ifdef _WINDOWS
#include <winsock.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <unistd.h>
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>


enum { kSigelMasterRegPort = 6789,
       kSuicidalRequest    = 13
     };


int main (int argc, char *argv[])
{ struct sockaddr_in   sad;
  struct hostent      *ptrh;
  struct protoent     *ptrp;
#ifdef _WINDOWS
	int	socke;
	char	msg[64];
#else
  int                  socke,
                       msg;
#endif
  char                 locHostName[256],
                       sigHostName[256];

#ifdef _WINDOWS
	// negotiate the socket version to use
	WSADATA	SocketData;
	if( WSAStartup( MAKEWORD(1,1), &SocketData)){
		fprintf( stderr, "Error while negotiating the socket version to use." );
		exit(1);
	}
#endif

  if ( gethostname(locHostName, 255) )
  { fprintf(stderr, "ERR:   gethostname() failed..\n\n");
    exit(1);
  }

  /* init sockaddr struct: using Internet family, port kSigelMasterRegPort */
  memset((char *)&sad, 0, sizeof(struct sockaddr_in));
  sad.sin_family = AF_INET;
  sad.sin_port = htons(kSigelMasterRegPort);

  /* now get hostname where the sigel master is running */
  if (argc <= 1)
  { fprintf(stderr, "Please submit the hostname where the sigel master is running !\n\n\tmanage_dyn_slave <master_host_name>\n\n");
#ifdef _WINDOWS
	 WSACleanup();
#endif	
    exit(1);
  }

  strncpy(sigHostName, argv[1], 255);

  /* make IP-Address with hostname */
  ptrh = gethostbyname(sigHostName);
  if (ptrh == NULL)
  { fprintf(stderr, "ERR:   invalid host: %s\n", sigHostName);
#ifdef _WINDOWS
	 WSACleanup();
#endif	
    exit(1);
  }

  memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

  /* map TCP protocol number */
  if (((int)(ptrp = getprotobyname("tcp"))) == 0)
  { fprintf(stderr, "ERR:   Can't map 'tcp' to a protocol number\n");
#ifdef _WINDOWS
	 WSACleanup();
#endif	
    exit(1);
  }

  /* finally create the socket */
  socke = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
  if (socke < 0)
  {  fprintf(stderr, "ERR:   Can't create socket\n");
#ifdef _WINDOWS
	  WSACleanup();
#endif	
     exit(1);
  }

  if (connect(socke, (struct sockaddr *)&sad, sizeof(sad)) < 0)
  {  fprintf(stderr, "ERR:  Can't connect to server\n");
#ifdef _WINDOWS
	  WSACleanup();
#endif	
     exit(1);
  }

  /* send our local hostname to the master */
  send(socke, locHostName, sizeof(locHostName), 0);
  fprintf(stderr, "Registered local host \"%s\" with SIGEL master server \"%s\".\n", locHostName, sigHostName);
  fprintf(stderr, "Waiting for answer from server to exit..\n\n");

  /* now wait for some message; we'll quit automatically when the server has sent something to us
   */
#ifdef _WINDOWS
	recv(socke, msg, 64, 0);
	closesocket(socke);
	WSACleanup();
#else	
  recv(socke, &msg, sizeof(msg), 0);
  close(socke);
#endif


  fprintf(stderr, "<manage_dyn_slave> is exiting.\n\n");

  /* that's all for now, folks ! */
  exit(0);
}
