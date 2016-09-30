/*  netshaper.c
 *
 *	Copyright (c) 2003  Bertera Pietro 
 *	e-mail:		<dr.iggy@iol.it>	<p.bertera@valtellinux.it>
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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <asm/types.h>

#include "netshaper.h"


#define DEFAULT_TIMEOUT 10000	/* milliseconds */


__u32
host_from_string (char *string)
{
  __u32 ip;
  char *slashptr = strchr (string, '/');
  struct hostent *host;

  if (slashptr)
    *slashptr = '\0';
  ip = inet_addr (string);
  if (ip == (__u32) - 1)
    {
      host = gethostbyname (string);
      if (host)
	ip = *(__u32 *) (host->h_addr);
      if (ip == (__u32) - 1)
	ip = 0;
    }
  if (slashptr)
    *slashptr = '/';
  return ip;
}


__u32
mask_from_string (char *string)
{
  __u32 ip;
  int i;
  char *slashptr = strchr (string, '/');
  static char trash[4];

  if (!slashptr)
    return 0xffffffff;
  string = slashptr + 1;
  /* check for simple "/24" form */
  if (strlen (string) < 5 && isdigit (string[0])
      && sscanf (string, "%i%s", &i, trash) == 1)
    {
      if (i > 32)
	return 0;		/* error */
      return htonl (~((1 << (32 - i)) - 1));
    }
  /* otherwise, dotted form */
  ip = inet_addr (string);
  if (ip == (__u32) - 1)
    {
      return 0;
    }
  return ip;
}


volatile void
help (char *prgname)
{
  fprintf (stderr, "%s: Usage: \"%s [options] bandwidth\n"
	   "	\"-s <host>\" specify a source host\n"
	   "	\"-d <host>\" specify a destination host\n"
	   "  		 <host> can specify a mask like \"/24\" or \"/255.255.255.0\"\n"
	   "	\"-sport <host>\" specify a source port\n"
	   "	\"-dport <host>\" specify a destination port\n"
	   "	\"-p <protocol>\" specify a protocol (udp and tcp only supported\n"
	   "	\"-i <iface>\" specify a input interface\n"
	   "	\"-o <iface>\" specify a output interface\n"
	   "	\"-t <timeout>\" specify a timeout of enqueued packets\n"
	   "Note: the bandwidth is in Bytes/seconds\n"
	   "see /proc/netshaper for rules info\n", prgname, prgname);
  exit (1);
}

int
main (int argc, char **argv)
{
  struct netshaper_userinfo info;
  char *ifname = "netshaper";
  char *prgname = argv[0];
  struct ifreq req;
  int i, c, sock = socket (AF_INET, SOCK_RAW, IPPROTO_RAW);

  if (sock < 0)
    {
      fprintf (stderr, "%s: socket(): %s\n", prgname, strerror (errno));
      exit (1);
    }

  c = 0;
  strcpy (req.ifr_name, ifname);
  req.ifr_data = (caddr_t) & info;

  info.out = NULL;
  info.in = NULL;
  info.sport = 0;
  info.dport = 0;
  info.daddr = host_from_string ("0.0.0.0");
  info.saddr = host_from_string ("0.0.0.0");
  info.dmask = mask_from_string ("0.0.0.0");
  info.smask = mask_from_string ("0.0.0.0");
  info.max_time = DEFAULT_TIMEOUT;
  info.protocol_type = 0;

  for (i = 1; i < argc; i)
    {
      if (strcmp (argv[i], "-d") == 0)
	{
	  info.daddr = host_from_string (argv[i + 1]);
	  info.dmask = mask_from_string (argv[i + 1]);
	  printf ("daddr: %s\n", argv[i + 1]);
	  c = c + 2;
	}
      if (strcmp (argv[i], "-s") == 0)
	{
	  info.saddr = host_from_string (argv[i + 1]);
	  info.smask = mask_from_string (argv[i + 1]);
	  printf ("saddr: %s\n", argv[i + 1]);
	  c = c + 2;
	}
      if (strcmp (argv[i], "-dport") == 0)
	{
	  info.dport = htons (atoi (argv[i + 1]));
	  printf ("dport: %s\n", argv[i + 1]);
	  c = c + 2;
	}
      if (strcmp (argv[i], "-sport") == 0)
	{
	  info.sport = htons (atoi (argv[i + 1]));
	  printf ("sport: %s\n", argv[i + 1]);
	  c = c + 2;
	}
      if (strcmp (argv[i], "-i") == 0)
	{
	  info.in = argv[i + 1];
	  printf ("in: %s\n", argv[i + 1]);
	  c = c + 2;
	}
      if (strcmp (argv[i], "-o") == 0)
	{
	  info.out = argv[i + 1];
	  printf ("out: %s\n", argv[i + 1]);
	  c = c + 2;
	}
      if (strcmp (argv[i], "-p") == 0)
	{
	  if (strcmp (argv[i + 1], "tcp") == 0)
	    {
	      info.protocol_type = IPPROTO_TCP;
	    }
	  if (strcmp (argv[i + 1], "udp") == 0)
	    {
	      info.protocol_type = IPPROTO_UDP;
	    }
	  printf ("protocol: %s\n", argv[i + 1]);
	  c = c + 2;
	}
      if (strcmp (argv[i], "-t") == 0)
	{
	  info.max_time = atoi (argv[i + 1]);
	  printf ("timeout: %s\n", argv[i + 1]);
	  c = c + 2;
	}
      i++;
      i++;
    }

  if (argc != c + 2)
    {
      help (prgname);
    }

  info.bytes_per_second = atoi (argv[argc - 1]);


  if (ioctl (sock, SIOCNETSHAPERSET, &req) < 0)
    {
      fprintf (stderr, "errore: ioctl():\n");
      exit (1);
    }

  return 0;
}
