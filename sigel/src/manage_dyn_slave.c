/*  <manage_dyn_slave.c>  --  Part of the SIGEL project
 *
 *  This little program has two purposes:
 *     1)  Register the local host.
 *     2)  Wait until the sigel master ends.
 *
 *  If the sigel master is launched in dynamic-evolve mode it listens to
 *  incoming connections on port 6789.
 *
 *  Once connected this program transmits the name of the local host,
 *  that's all. Thus the sigel master knows our local host has a sigel_slave
 *  binary installed in /tmp/_SIGEL_EVOLUTION_TEMP
 *
 *  The sigelDynClient script makes sure the latter is the case, and then
 *  starts this program.
 *
 *  When the sigel master ends, this program exits,
 *  and sigelDynClient removes the /tmp/_SIGEL_EVOLUTION_TEMP directory.
 *
 *  Author:  Jan Barnholt  <jan-b@uni.de>, <jan.barnholt@epost.de>
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <unistd.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

enum { kSigelMasterRegPort = 6789,
       kSuicidalRequest    = 13
     };

int main (int argc, char *argv[])
{ struct sockaddr_in   sad;
  struct hostent      *masterHost;
  struct protoent     *tcpProtocol;
  int                  masterSocket,
                       msg;
  char                 locHostName[256],
                       sigHostName[256];

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
    exit(1);
  }

  strncpy(sigHostName, argv[1], 255);

  /* make IP-Address with hostname */
  masterHost = gethostbyname(sigHostName);
  if (masterHost == NULL)
  { fprintf(stderr, "ERR:   invalid host: %s\n", sigHostName);
    exit(1);
  }

  memcpy(&sad.sin_addr, masterHost->h_addr, masterHost->h_length);

  /* map TCP protocol number */
  tcpProtocol = getprotobyname("tcp");
  if (tcpProtocol == NULL)
  { fprintf(stderr, "ERR:   Can't map 'tcp' to a protocol number\n");
    exit(1);
  }

  /* finally create the socket */
  masterSocket = socket(PF_INET, SOCK_STREAM, tcpProtocol->p_proto);
  if (masterSocket < 0)
  {  fprintf(stderr, "ERR:   Can't create socket\n");
     exit(1);
  }

  if (connect(masterSocket, (struct sockaddr *)&sad, sizeof(sad)) < 0)
  {  fprintf(stderr, "ERR:  Can't connect to server\n");
     exit(1);
  }

  /* send our local hostname to the master */
  send(masterSocket, locHostName, sizeof(locHostName), 0);
  fprintf(stderr, "Registered local host \"%s\" with SIGEL master server \"%s\".\n", locHostName, sigHostName);
  fprintf(stderr, "Waiting for answer from server to exit..\n\n");

  /* now wait for some message; we'll quit automatically when the server has sent something to us
   */
  recv(masterSocket, &msg, sizeof(msg), 0);
  close(masterSocket);

  fprintf(stderr, "<manage_dyn_slave> is exiting.\n\n");

  /* that's all for now, folks ! */
  exit(0);
}
