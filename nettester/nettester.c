/*  tester.c Bertera Pietro dr.iggy@iol.it
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */



#include <stdio.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>

#define MAX_BUFF 1024
#define MAXCONN 1
#define SERVER 1
#define CLIENT 2

volatile void
help (char *prgname)
{
  fprintf (stderr, "%s: Usage: \"%s [options]"
	   "	\"-s \" Server Mode\n"
	   "	\"-u \" udp\n"
	   "	\"-d <ip>\" ip destinazione in client mode"
	   "	\"-p <port>\" porta\n", prgname);
  exit (1);
}

void
addr_initialize (struct sockaddr_in *indirizzo, int port, long IPaddr)
{
  indirizzo->sin_family = AF_INET;
  indirizzo->sin_port = htons ((u_short) port);
  indirizzo->sin_addr.s_addr = IPaddr;
}

int
main (int argc, char **argv)
{
  long ip = 0;
  int port, i, n;
  int sock_type = SOCK_STREAM;
  char *tmp;
  int mode = CLIENT;
  char *prgname = argv[0];
  int sd, new_sd;
  struct sockaddr_in server_addr;
  struct sockaddr_in client_addr;
  int client_len = sizeof (client_addr);
  char buff[MAX_BUFF];
  port = 0;

  for (i = 1; i < argc; i)
    {

      if (strcmp (argv[i], "-d") == 0)
	{
	  ip = inet_addr (argv[i + 1]);
	  i++;
	  i++;
	  continue;
	}

      if (strcmp (argv[i], "-p") == 0)
	{
	  port = atoi (argv[i + 1]);
	  i++;
	  i++;
	  continue;
	}

      if (strcmp (argv[i], "-s") == 0)
	{
	  mode = SERVER;
	  i++;
	  continue;
	}

      if (strcmp (argv[i], "-u") == 0)
	{
	  sock_type = SOCK_DGRAM;
	  i++;
	  continue;
	}
      help (prgname);
    }

  if ((mode == CLIENT) && (!port || !ip))
    help (prgname);
  if ((mode == SERVER) && (!port))
    help (prgname);

  switch (mode)
    {
    case SERVER:
      addr_initialize (&server_addr, port, INADDR_ANY);
      if ((sd = socket (AF_INET, sock_type, 0)) < 0)
	{
	  fprintf (stderr, "%s", "Non posso creare il socket\n");
	  return -1;
	}

      if ((bind (sd, (struct sockaddr *) &server_addr, sizeof (server_addr)))
	  < 0)
	{
	  fprintf (stderr, "%s", "Non posso bindare\n");
	  return -1;
	}

      if (sock_type == SOCK_STREAM)
	{
	  listen (sd, MAXCONN);
	  if ((new_sd =
	       accept (sd, (struct sockaddr *) &client_addr,
		       &client_len)) < 0)
	    {
	      fprintf (stderr, "%s", "non posso fare l'accept\n");
	      return -1;
	    }

	  while ((n = read (new_sd, buff, MAX_BUFF)) > 0)
	    if (write (1, buff, n) != n)
	      fprintf (stderr, "%s", "Errore di scrittura sul socket\n");
	}

      if (sock_type == SOCK_DGRAM)
	{
	  while ((n = read (sd, buff, MAX_BUFF)) > 0)
	    if (write (1, buff, n) != n)
	      fprintf (stderr, "%s", "Errore di scrittura sul socket\n");
	}
      break;
    case CLIENT:

      addr_initialize (&server_addr, port, ip);
      if ((sd = socket (AF_INET, sock_type, 0)) < 0)
	{
	  fprintf (stderr, "%s", "Non posso creare il socket\n");
	  return -1;
	}
      if ((connect
	   (sd, (struct sockaddr *) &server_addr, sizeof (server_addr))) < 0)
	{
	  fprintf (stderr, "%s", "Non posso fare la connect\n");
	  return -1;
	}


      while ((n = read (0, buff, MAX_BUFF)) > 0)
	if (write (sd, buff, n) != n)
	  fprintf (stderr, "%s", "Errore di scrittura sul socket\n");


      break;
    }

  return 0;
}
