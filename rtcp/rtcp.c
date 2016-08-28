/*             Sat Mar  8 00:05:40 2003
 *
 *					Bertera Pietro 
 *		e-mail: pietro@bertera.it
 *
 *					compile with:
 * gcc -Wall -DMODULE -D__KERNEL__ -c rtcp.c -I/usr/src/linux/include
 *
 *					run with:
 * insmod rtcp.o
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
#define __KERNEL__
#endif
#ifndef MODULE
#define MODULE
#endif

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/skbuff.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/config.h>
#include <linux/in.h>

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("Bertera Pietro");
MODULE_DESCRIPTION ("Reverse TCP kernel module");

char *ip;
MODULE_PARM (ip, "s");

__u32
in_aton (const char *str)
{
  unsigned long l;
  unsigned int val;
  int i;

  l = 0;
  for (i = 0; i < 4; i++)
    {
      l <<= 8;
      if (*str != '\0')
	{
	  val = 0;
	  while (*str != '\0' && *str != '.')
	    {
	      val *= 10;
	      val += *str - '0';
	      str++;
	    }
	  l |= val;
	  if (*str != '\0')
	    str++;
	}
    }
  return (__constant_htonl (l));
}

unsigned int
hook_fn (unsigned int hooknum,
	 struct sk_buff **skb_p,
	 const struct net_device *in,
	 const struct net_device *out, int (*okfn) (struct sk_buff *))
{
  int retval = NF_ACCEPT;
  struct sk_buff *skb = (*skb_p);
  struct iphdr *iph = NULL;
  struct tcphdr *tcph = NULL;


  __u32 saddr;
  __u32 daddr;
  __u32 magic;
  __u8 protocol_type;
  __u16 flag;
  iph = skb->nh.iph;

  saddr = iph->saddr;
  daddr = iph->daddr;

  protocol_type = iph->protocol;

  if (protocol_type == IPPROTO_TCP)
    {
      tcph = (struct tcphdr *) ((__u32 *) iph + iph->ihl);
      magic = in_aton (ip);
      if ((saddr == magic) || (daddr == magic))
	{
	  flag = tcph->fin;
	  tcph->fin = tcph->syn;
	  tcph->syn = flag;
	}
    }
  return retval;
}

struct nf_hook_ops pre_hook_ops = {
  hook:hook_fn,
  pf:PF_INET,
  hooknum:NF_IP_PRE_ROUTING,
  priority:NF_IP_PRI_FIRST,
};

struct nf_hook_ops post_hook_ops = {
  hook:hook_fn,
  pf:PF_INET,
  hooknum:NF_IP_POST_ROUTING,
  priority:NF_IP_PRI_FIRST,
};

static int init_status_flag;

#define PRE_HOOK_REGISTERED	0x08
#define POST_HOOK_REGISTERED	0x20

void
cleanup_module (void)
{
  if (init_status_flag & PRE_HOOK_REGISTERED)
    nf_unregister_hook (&pre_hook_ops);

  if (init_status_flag & POST_HOOK_REGISTERED)
    nf_unregister_hook (&post_hook_ops);
}

int
init_module (void)
{
  int result;
  if (!ip)
    {
      printk ("Error: missing end-host ip.\n");
      printk ("Usage: insmod rtcp.o ip=x.x.x.x \n\n");
      return -ENXIO;
    }
  result = nf_register_hook (&pre_hook_ops);
  if (result < 0)
    {
      printk (KERN_ERR "can't register netfilter hook prerouting");
      cleanup_module ();
      return result;
    }
  init_status_flag |= PRE_HOOK_REGISTERED;

  result = nf_register_hook (&post_hook_ops);
  if (result < 0)
    {
      printk (KERN_ERR "can't register netfilter hook postrouting");
      cleanup_module ();
      return result;
    }
  init_status_flag |= POST_HOOK_REGISTERED;
  printk ("Rtcp run ;)");

  return 0;
}
