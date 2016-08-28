/*
 *					Bertera Pietro 
 *		e-mail: p.bertera@valtellinux.it dr.iggy@iol.it
 *
 *					compile with:
 * gcc kechoctrl.c -o kechoctrl
 *
 *					run with:
 * kechoctrl [string]
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

#include "kecho_mod.h"

int main(int argc, char **argv)
{
    struct my_userinfo info;
    char *ifname = "kecho";
    struct ifreq req;
    int i, sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);

    if (sock < 0) {
        fprintf(stderr, "%s: socket():\n");
        exit(1);
    }

    strcpy(req.ifr_name, ifname);
    req.ifr_data = (caddr_t)&info;
	
	info.string=argv[1];

	if (ioctl(sock, PRINT, &req)<0) {
	    fprintf(stderr, "%s: ioctl():\n");
	    exit(1);
	}
	return 0;
}