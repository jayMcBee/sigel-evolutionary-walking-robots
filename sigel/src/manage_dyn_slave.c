/*  <manage_dyn_slave.c>  --  Part of the SIGEL project
 *
 *  This little program has two purposes:
 *     1)  Register the local host.
 *     2)  Wait until the sigel master ends.
 *
 *  If the sigel master is launched in dynamic-evolve mode it listens to
 *  incoming connections on port 6789.
 *
 *  Once connected this program transmits the name of the local host and
 *  the number of slaves the master may run on it at the same time,
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

enum { kSigelMasterRegPort = 6789 };

int main (int argc, char *argv[])
{ struct addrinfo      hints,
                      *masterAddresses,
                      *address;
  int                  masterSocket,
                       lookupResult,
                       msg;
  char                 locHostName[256],
                       sigHostName[256],
                       masterPort[8],
                       registration[512];

  if ( gethostname(locHostName, 255) )
  { fprintf(stderr, "ERR:   gethostname() failed..\n\n");
    exit(1);
  }

  /* now get hostname where the sigel master is running */
  if (argc <= 2)
  { fprintf(stderr, "Please submit the hostname where the sigel master is running and the number of slaves for this host !\n\n\tmanage_dyn_slave <master_host_name> <number_of_slaves>\n\n");
    exit(1);
  }

  if (atoi(argv[2]) < 1)
  { fprintf(stderr, "ERR:   the number of slaves must be 1 or more: %s\n", argv[2]);
    exit(1);
  }

  strncpy(sigHostName, argv[1], 255);

  /* the master's IPv4 and IPv6 addresses, or the address given */
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  snprintf(masterPort, sizeof(masterPort), "%d", kSigelMasterRegPort);

  lookupResult = getaddrinfo(sigHostName, masterPort, &hints, &masterAddresses);
  if (lookupResult != 0)
  { fprintf(stderr, "ERR:   invalid host: %s (%s)\n", sigHostName, gai_strerror(lookupResult));
    exit(1);
  }

  /* connect to the first address that answers */
  masterSocket = -1;
  for (address = masterAddresses; address != NULL; address = address->ai_next)
  { masterSocket = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
    if (masterSocket < 0)
    { continue;
    }

    if (connect(masterSocket, address->ai_addr, address->ai_addrlen) == 0)
    { break;
    }

    close(masterSocket);
    masterSocket = -1;
  }

  freeaddrinfo(masterAddresses);

  if (masterSocket < 0)
  {  fprintf(stderr, "ERR:  Can't connect to server\n");
     exit(1);
  }

  /* send our local hostname and the number of slaves to the master */
  memset(registration, 0, sizeof(registration));
  snprintf(registration, sizeof(registration), "%s %s", locHostName, argv[2]);
  send(masterSocket, registration, sizeof(registration), 0);
  fprintf(stderr, "Registered local host \"%s\" with SIGEL master server \"%s\", number of slaves: %s.\n", locHostName, sigHostName, argv[2]);
  fprintf(stderr, "Waiting for answer from server to exit..\n\n");

  /* wait until the master ends; recv returns when the connection closes */
  recv(masterSocket, &msg, sizeof(msg), 0);
  close(masterSocket);

  fprintf(stderr, "<manage_dyn_slave> is exiting.\n\n");

  /* that's all for now, folks ! */
  exit(0);
}
