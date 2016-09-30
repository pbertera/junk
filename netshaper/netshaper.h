/*  netshaper.h
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
 
#ifndef __NETSHAPER_H__
#define __NETSHAPER_H__



#define SIOCNETSHAPERSET SIOCDEVPRIVATE
#define SIOCNETSHAPERGET (SIOCDEVPRIVATE+1)


struct netshaper_userinfo
{
  __u32 saddr;			/* ip sorgente */
  __u32 daddr;			/* ip destinazione */
  __u32 smask;			/* maschera sorgente */
  __u32 dmask;			/* maschera destinazione */
  __u8 protocol_type;		/* tipo di protocolllo */
  __u16 dport;			/* porta destinazione */
  __u16 sport;			/* porta sorgente */
  char *in;			/* interfaccia di rete di input */
  char *out;			/* interfaccia di rete di output */
  int bytes_per_second;		/* banda (in B/SECONDI) */
  int max_time;			/* massima dimensione della coda (in jiffies) */
};


#endif
