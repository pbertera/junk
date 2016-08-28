/*
 *					Bertera Pietro 
 *		e-mail: p.bertera@valtellinux.it dr.iggy@iol.it
 *
 *					compile with:
 * gcc -Wall  -c kecho.c -I/usr/src/linux/include
 *
 *					run with:
 * insmod kecho.o
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
 
#ifndef __KERNEL__
#  define __KERNEL__
#endif
#ifndef MODULE
#  define MODULE
#endif

#include <linux/module.h>
#include <linux/kernel.h> 
#include <linux/types.h>  

#include <linux/netdevice.h>   
#include <linux/etherdevice.h> 

#include <asm/uaccess.h>
#include "kecho_mod.h"

MODULE_LICENSE("GPL");

int my_ioctl(struct net_device *dev, struct ifreq *ifr, int cmd)
{
	struct my_userinfo info,
    *uptr = (struct my_userinfo *)ifr->ifr_data;
   

    if (!suser()) return -EPERM;

   
    if (!access_ok(VERIFY_READ, uptr, sizeof(*uptr)))
        return -EFAULT;
    copy_from_user(&info, uptr, sizeof(info));

    switch(cmd) {
      case PRINT: 
	printk("Sento una voce dallo UserSpace: %s\n", info.string);

	    return 0;

      case REPRINT: 
		return 0;

      default:
        return -ENOTTY;
    }
    return 0;
}

int my_init_dev(struct net_device *dev)
{
    dev->do_ioctl = my_ioctl;
    return 0;
}

struct net_device my_dev = {
    name: "kecho",
    init: my_init_dev,
}; 

void cleanup_module(void)
{
        unregister_netdev(&my_dev);
}

int init_module(void)
{
   int result;    

    result = register_netdev(&my_dev);
    if (result) {
        printk(KERN_ERR "kecho: can't register netdevice");
        cleanup_module();
        return result;
    }
    
    return 0; 
}