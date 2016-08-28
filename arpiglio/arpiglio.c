/* arpiglio.c
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
#include <linux/if_ether.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#define IP_ALEN 4
#define IP_PROTO_TYPE 0x0800
/* ARP protocol opcodes. (da if_arp.h) */
#define	ARPOP_REQUEST	1	/* ARP request                  */
#define	ARPOP_REPLY	2	/* ARP reply                    */
#define	ARPOP_RREQUEST	3	/* RARP request                 */
#define	ARPOP_RREPLY	4	/* RARP reply                   */

/* ARP protocol HARDWARE identifiers. (da if_arp.h) */
#define ARPHRD_ETHER 	1	/* Ethernet 10Mbps              */
/* 
 * Formato dell'intestazione di un pacchetto ARP:
 * 
 * 0        8       16      24       31
 * +----------------+----------------+
 * | hw type        |  protocol type |
 * +--------+-------+----------------+
 * | hlen   | plen  |  operation     |
 * +--------+-------+----------------+
 * | src hw address                  |
 * +----------------+----------------+
 * | src hw address | src ip         |
 * +----------------+----------------+
 * | src ip         | dst hw address +
 * +----------------+----------------+
 * |  dst hw adress                  |
 * +---------------------------------+
 * |  dst ip                         |
 * +---------------------------------+
 */

struct ether_frame
{
/* Intestazione del frame Ethernet: */
  u_char eth_dst_hw_addr[ETH_ALEN];	/* indirizzo hw di dest. del frame (6 ottetti) */
  u_char eth_src_hw_addr[ETH_ALEN];	/* indirizzo hw di src. del frame  (6 ottetti) */
  u_short frame_type;		/* id protocollo contenuto nel frame (2 ottetti) */

/* Intestazione del pacchetto arp: */
  u_short arp_hw_id;		/* id del tipo di hw in arp (ethernet) (2 ottetti) */
  u_short arp_proto_type;	/* id del protocollo allo strato superiore (2 ottetti) */
  u_char arp_hw_addr_len;	/* lunghezza dell' indirizzo hw (1 ottetto) */
  u_char arp_proto_addr_len;	/* lunghezza dell' indirizzo del protocollo (1 ottetto) */
  u_short arp_opcode;		/* operazione (2 ottetti) */
  u_char arp_src_hw_addr[ETH_ALEN];	/* indirizzo hw sorgente (6 ottetti) */
  u_char arp_src_proto_addr[IP_ALEN];	/* indirizzo ip sorgente (4 ottetti) */
  u_char arp_dst_hw_addr[ETH_ALEN];	/* indirizzo hw destinatatrio (6 ottetti) */
  u_char arp_dst_proto_addr[IP_ALEN];	/* indirizzo ip destinatario (4 ottetti) */
  u_char padding[18];		/* padding */
};

void
help (char *prgname)
{
  fprintf (stderr, "%s: Usage: \"%s [options] \n"
	   "	\"-eda <hw_address>\" Ethernet Destination Address: indirizzo hw destinazione del frame ethernet\n"
	   "	\"-esa <hw_address>\" Ethernet Source Address: indirizzo hw sorgente del frame ethernet\n"
	   "	\"-ash <hw_address>\" Arp Source Hardware: indirzzo hw sorgente nel pacchetto (r)arp\n"
	   "	\"-adh <hw_address>\" Arp Destination Hardware: indirzzo hw destinazione nel pacchetto (r)arp\n"
	   "	\"-psa <ip_address>\" Protocol Source Address: indirzzo IP sorgente nel pacchetto (r)arp\n"
	   "	\"-pda <ip_address>\" Protocol Destination Address: indirzzo IP destinazione nel pacchetto (r)arp\n"
	   "	\"-dev <eth_dev>\" Ethernet Device: dispositivo di rete su cui inviare il datagram\n"
	   "	\"-t <arp_type>\" Arp type: specifica il tipo di arp:\n"
	   "	\"     areq\" Arp request\n"
	   "	\"     rreq\" Rarp request\n"
	   "	\"     arep\" Arp response\n"
	   "	\"     rrep\" Rarp response\n", prgname, prgname);
  exit (1);
}


int
get_hw_addr (u_char * buf, char *str)
{

  int i;
  char c, val = 0;

  for (i = 0; i < ETH_ALEN; i++)
    {
      if (!(c = tolower (*str++)))
	return -1;
      if (isdigit (c))
	val = c - '0';
      else if (c >= 'a' && c <= 'f')
	val = c - 'a' + 10;
      else
	return -1;

      *buf = val << 4;
      if (!(c = tolower (*str++)))
	return -1;
      if (isdigit (c))
	val = c - '0';
      else if (c >= 'a' && c <= 'f')
	val = c - 'a' + 10;
      else
	return -1;

      *buf++ |= val;

      if (*str == ':')
	str++;
    }
  return 1;
}

int
get_ip_addr (struct in_addr *in_addr, char *str)
{

  struct hostent *hostp;

  in_addr->s_addr = inet_addr (str);
  if (in_addr->s_addr == -1)
    {
      if ((hostp = gethostbyname (str)))
	bcopy (hostp->h_addr, in_addr, hostp->h_length);
      else
	{
	  return -1;
	}
    }
  return 1;
}



int
main (int argc, char **argv)
{
  struct in_addr src_ip, dst_ip;
  struct ether_frame frame;
  struct sockaddr sa;
  int sock, i, c;

  frame.frame_type = htons (ETH_P_ARP);	/* default arp */
  frame.arp_hw_id = htons (ARPHRD_ETHER);	/* default ethernet */
  frame.arp_proto_type = htons (IP_PROTO_TYPE);	/*  */
  frame.arp_hw_addr_len = ETH_ALEN;
  frame.arp_proto_addr_len = IP_ALEN;
  frame.arp_opcode = htons (ARPOP_REPLY);	/* default arp replay */

  bzero (frame.padding, 18);
  c = -1;
  for (i = 1; i < argc; i)
    {
      /* tipo di protocollo */
      if (strcmp (argv[i], "-t") == 0)
	{
	  if (strcmp (argv[i + 1], "areq") == 0)
	    {			// arp request
	      frame.frame_type = htons (ETH_P_ARP);
	      frame.arp_opcode = htons (ARPOP_REQUEST);
	    }
	  if (strcmp (argv[i + 1], "rreq") == 0)
	    {			// rarp request
	      frame.frame_type = htons (ETH_P_RARP);
	      frame.arp_opcode = htons (ARPOP_RREQUEST);
	    }

	  if (strcmp (argv[i + 1], "arep") == 0)
	    {			// arp reply
	      frame.frame_type = htons (ETH_P_ARP);
	      frame.arp_opcode = htons (ARPOP_REPLY);
	    }
	  if (strcmp (argv[i + 1], "rrep") == 0)
	    {			// rarp reply
	      frame.frame_type = htons (ETH_P_ARP);
	      frame.arp_opcode = htons (ARPOP_RREPLY);
	    }
	  c = c + 2;
	}

      if (strcmp (argv[i], "-eda") == 0)	//ethernet destination address
	{
	  if ((get_hw_addr (frame.eth_dst_hw_addr, argv[i + 1])) < 0)
	    {
	      fprintf (stderr, "indirizzo MAC invalido (-eda)\n");
	      exit (-1);
	    }
	  c = c + 2;
	}

      if (strcmp (argv[i], "-esa") == 0)	//ethernet source address
	{
	  if ((get_hw_addr (frame.eth_src_hw_addr, argv[i + 1])) < 0)
	    {
	      fprintf (stderr, "indirizzo MAC invalido (-esa)\n");
	      exit (-1);
	    }
	  c = c + 2;
	}
      if (strcmp (argv[i], "-ash") == 0)	// arp source hardware (address)
	{
	  if ((get_hw_addr (frame.arp_src_hw_addr, argv[i + 1])) < 0)
	    {
	      fprintf (stderr, "indirizzo MAC invalido (-ash)\n");
	      exit (-1);
	    }
	  c = c + 2;
	}

      if (strcmp (argv[i], "-adh") == 0)	// arp destination hardware (address)
	{
	  if ((get_hw_addr (frame.arp_dst_hw_addr, argv[i + 1])) < 0)
	    {
	      fprintf (stderr, "indirizzo MAC invalido (-adh)\n");
	      exit (-1);
	    }
	  c = c + 2;
	}

      if (strcmp (argv[i], "-psa") == 0)	// protocol source address
	{
	  if (get_ip_addr (&src_ip, argv[i + 1]) < 0)
	    {
	      fprintf (stderr, "indirizzo invalido (-psa)\n");
	      exit (-1);
	    }
	  memcpy (frame.arp_src_proto_addr, &src_ip, IP_ALEN);
	  c = c + 2;
	}

      if (strcmp (argv[i], "-pda") == 0)	// protocol source address
	{
	  if (get_ip_addr (&dst_ip, argv[i + 1]) < 0)
	    {
	      fprintf (stderr, "indirizzo invalido (-pda)\n");
	      exit (-1);
	    }
	  memcpy (frame.arp_dst_proto_addr, &dst_ip, IP_ALEN);
	  c = c + 2;
	}

      if (strcmp (argv[i], "-dev") == 0)	// protocol source address
	{
	  strcpy (sa.sa_data, argv[i + 1]);
	  c = c + 2;
	}

      i++;
      i++;
    }
  if (argc != c + 2)
    {
      printf ("argc %d\n", argc);
      help (argv[0]);
    }
  sock = socket (AF_INET, SOCK_PACKET, htons (ETH_P_ALL));
  if (sendto (sock, &frame, sizeof (frame), 0, &sa, sizeof (sa)) < 0)
    {
      fprintf (stderr, "errore in sendto");
      exit (-1);
    }
}
