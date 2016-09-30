/*  netshaper_mod.c
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


#ifndef __KERNEL__
#define __KERNEL__
#endif

#ifndef MODULE
#define MODULE
#endif

#include <linux/config.h>
#ifndef UTS_RELEASE
#  include <linux/version.h>
#endif

#ifndef KERNEL_VERSION
#  define KERNEL_VERSION(vers,rel,seq) ( ((vers)<<16) | ((rel)<<8) | (seq) )
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,0,0)
#  error "Questo kernel è troppo vecchio"
#  error LINUX_VERSION_CODE
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,5,0)
#  error "Questo kernel è troppo recente"
#endif

#undef LINUX_24
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,4,0)
#  define LINUX_24
#endif

#ifdef LINUX_24
#  ifndef CONFIG_NETFILTER
#    error "Non è abilitato il support per il netfilter del kernel"
#  endif
#  include <linux/netfilter.h>
#  include <linux/netfilter_ipv4.h>
#endif

#define NETSHAPE_PROC_DIR "netshaper"
#define NETSHAPER_PROC_FILE "rules"

#include <asm/uaccess.h>
#include <asm/byteorder.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/skbuff.h>
#include <linux/in.h>
#include <linux/proc_fs.h>
#include <linux/byteorder/generic.h>
#include <linux/timer.h>
#include "netshaper.h"

#define IN_QUEUE  1
#define OUT_QUEUE 2

MODULE_LICENSE ("GPL");

/* netshaper_packet è una struttura che rappresenta ogni singolo elemento di una coda 	*/
/* ogni elemento contiente il pacchetto reale, un riferimento al prossimo elemento e  	*/
/* una funzione per poter deaccodare il pacchetto 										*/

struct netshaper_packet
{
  struct sk_buff *skb;		/* pacchetto */
  struct netshaper_packet *next;	/* prossimo paccehtto nella coda */
  unsigned long timestamp;	/* tempo di permanenza nella coda */
  int (*okfn) (struct sk_buff *);	/* funzione per togliere dalla coda il pacchetto */
};

/* netshaper_queue modella una coda di netshaper_packet, contiene un puntatore all'		*/
/* inizio della coda e un puntatore alla fine, contiene anche una timer_list che serve 	*/
/* per togliere un pacchetto dalla coda nel momento opportuno, time_exc contiene  		*/
/* lo scarto di tempo dovuto alle approssimazioni dei pacchetti precedenti. lock è uno	*/
/* spinlock per effetuare mututa esclusione per evitare casi di deadlock quando si 		*/
/* lavora sulla coda */

struct netshaper_queue
{
  struct netshaper_packet *head;	/* testa della lista */
  struct netshaper_packet *tail;	/* coda della lista */
  struct timer_list timer;	/* per togliere dalla lista un pacchetto */
  unsigned long nexttick;	/* primo istante in cui si puo' inviare un pacchetto */
  int time_exc;			/* eccesso dovuto alla approssimazione */
  spinlock_t lock;		/* locking */
  struct netshaper_rule *rule;
};

/* netshaper_rule rapperesenta una regola in cui vengono inserite le caratteristiche 	*/
/* dei pacchetti, il bitrate (in bytes per tick) che devono avere i paccehtti che  		*/
/* rispettano queste caratteristiche e il tempo di permanenza massimo nella coda di ogni*/
/* pacchetto */

struct netshaper_rule
{				/* Struttura che forma la lista delle regole */
  __u32 saddr;			/* ip sorgente */
  __u32 daddr;			/* ip destinazione */
  __u32 smask;			/* netmask dell'ip sorgente */
  __u32 dmask;			/* netmask dell'ip destinazione */
  __u8 protocol_type;		/* tipo di protocolllo */
  __u16 dport;			/* porta destinazione */
  __u16 sport;			/* porta sorgente */
  struct net_device *in;
  struct net_device *out;
  struct netshaper_queue *out_queue;	/* coda con i pacchetti in uscita */
  struct netshaper_queue *in_queue;	/* coda con i pacchetti in ingresso */
  int bytes_per_tick;		/* banda (in B/tick) */
  int max_time;			/* massima dimensione della coda (in jiffies) */
  struct netshaper_rule *next;	/* prossima regola */
  int size;
  int payload;
};

struct netshaper_rule *first_rule;	/* prima regola */
spinlock_t netshaper_rule_lock;	/* locking sulle regole */
static struct proc_dir_entry *netshaper_proc_dir, *netshaper_proc_file;	/* /proc dir */

static int
netshaper_proc_read_file (char *page, char **start, off_t off, int count,
			  int *eof, void *data)
{
  int length = 0;
  long flags;
  char *proto = NULL;
  char *in = NULL;
  char *out = NULL;
  char *page_start = page;
  struct netshaper_rule *rule;
  
  spin_lock_irqsave (&netshaper_rule_lock, flags);
       length= sprintf (page,	"<?xml version=\"1.0\"?>\n"
      				"<!DOCTYPE rules\n"
      				"\t[\n"
      				"\t<!ELEMENT rule (saddr, sport, daddr, dport, smask, dmask, proto, inIface, outIface, band,length, size, data)>\n"
      				"\t<!ELEMENT saddr (#PCDATA)>\n"
      				"\t<!ELEMENT sport (#PCDATA)>\n"
       				"\t<!ELEMENT daddr (#PCDATA)>\n"
        			"\t<!ELEMENT dport (#PCDATA)>\n"
   				"\t<!ELEMENT smask (#PCDATA)>\n"
      				"\t<!ELEMENT dmask (#PCDATA)>\n"
      				"\t<!ELEMENT proto (#PCDATA)>\n"
      				"\t<!ELEMENT inIface (#PCDATA)>\n"
      				"\t<!ELEMENT outIface (#PCDATA)>\n"
      				"\t<!ELEMENT band (#PCDATA)>\n"
      				"\t<!ELEMENT length (#PCDATA)>\n"
      				"\t<!ELEMENT size (#PCDATA)>\n"
      				"\t<!ELEMENT data (#PCDATA)>\n"
				"\t]\n"
				">\n\n"
      );
     page += length;
     length= sprintf (page,"<rules>\n");
     page += length;
  for (rule = first_rule; rule; rule = rule->next)
    {
      if (rule->protocol_type == IPPROTO_TCP)
	proto = "tcp";

      if (rule->protocol_type == IPPROTO_UDP)
	proto = "udp";   
      
      if (!rule->protocol_type)
	      proto = "tcp";
      if(!rule->in->name)
	      in="";
      else in=rule->in->name;
      if(!rule->out->name)
	      out="";
      else out=rule->out->name;
     length = sprintf (page,	"\t<rule>\n"
      				"\t\t<saddr>%d.%d.%d.%d</saddr>\n"
      				"\t\t<sport>%d</sport>\n"
      				"\t\t<daddr>%d.%d.%d.%d</daddr>\n"
      				"\t\t<dport>%d</dport>\n"
      				"\t\t<smask>%d.%d.%d.%d</smask>\n" 
      				"\t\t<dmask>%d.%d.%d.%d</dmask>\n"
      				"\t\t<proto>%s</proto>\n"
      				"\t\t<inIface>%s</inIface>\n"
      				"\t\t<outIface>%s</outIface>\n"
      				"\t\t<band>%d</band>\n"
      				"\t\t<length>%d</length>\n"
      				"\t\t<size>%d</size>\n"
      				"\t\t<data>%d</data>\n"
      				"\t</rule>\n",
		 NIPQUAD (rule->saddr), __constant_ntohs (rule->sport),
                 NIPQUAD (rule->daddr), __constant_ntohs (rule->dport),
                 NIPQUAD (rule->smask), NIPQUAD (rule->dmask), proto,
		 in, out, HZ * rule->bytes_per_tick,
		 rule->max_time, rule->size, rule->payload);
     page += length;
          
    }
      length= sprintf (page,"</rules>\n");
     page += length;
  spin_unlock_irqrestore (&netshaper_rule_lock, flags);
  *eof = 1;
  return (page - page_start - off);
}

/* funzione per inserire un pacchetto in una coda */
int
netshaper_enqueue_packet (struct sk_buff *skb, struct netshaper_queue *queue,
			  int bytes_per_tick, int (*okfn) (struct sk_buff *))
                            
{

  struct netshaper_packet *pk;	/* puntatore al pacchetto vero e proprio */
  int timerflag = 0;		/* flag per il timer della coda */

  int size = skb->tail - skb->data;	/* dimensione del payload del pacchetto */
#ifdef DEBUGG
  printk ("nexttick: %li  jiffies: %li\n", queue->nexttick, jiffies);
#endif
  /* se il nexttick della regola è già passato accetta il pacchetto */
  if (queue->nexttick <= jiffies)
    {
      /* se è minore non aggiunge approssimazione */
      if (queue->nexttick < jiffies)
	{
	  queue->time_exc = 0;
	}
      /* viene aumentata la dimensione (non reale) del pacchetto per tenere   */
      /* conto dello scarto */
      size += queue->time_exc;
      /* viene calcolato il momento in cui deve essere tolto dalla coda in    */
      /* base al bitrate (in B/tick) */
      queue->nexttick = jiffies + size / bytes_per_tick;
      /* se c'è un'approssimazione viene aggiunta al campo apposito della coda */
      /* per potere fare in modo che il prossimo pacchetto ne tenga conto */
      queue->time_exc = size % bytes_per_tick;
#ifdef DEBUGG
      printk
	("Netshaper: Accetto senza accodare il pacchetto:saddr: %d.%d.%d.%d\n",
	 NIPQUAD (skb->nh.iph->saddr));
      printk ("daddr: %d,%d,%d,%d\n", NIPQUAD (skb->nh.iph->daddr));
#endif

      /* viene ritornato all'hook il valore per accettare il pacchetto */
      return NF_ACCEPT;
    }
  /* il pacchetto va allocato nella coda */
  /* alloca la lo spazio per aggiungere il pacchetto alla coda */
  pk = kmalloc (sizeof (struct netshaper_packet), GFP_ATOMIC);
  if (!pk)
    {				/* errore in kmalloc */
      printk ("netshaper: non posso allocare\n");
      kfree_skb (skb);
      /* ritorna all'hook il valore per tenere in sospeso il pacchetto */
      return NF_DROP;
    }
#ifdef DEBUGG
  printk ("Aggiungo il pacchetto alla coda: saddr: %d.%d.%d.%d\n",
	  NIPQUAD (skb->nh.iph->saddr));
  printk ("daddr: %d,%d,%d,%d\n", NIPQUAD (skb->nh.iph->daddr));
#endif
  /* Inizializza la struttura netshaper_packet relativa al pacchetto */
  pk->skb = skb;		/* il vero pacchetto */
  pk->okfn = okfn;		/* la funzione per toglierlo dalla coda */
  pk->next = NULL;		/* il prossimo (NULL  perchè è l'ultimo) */
  pk->timestamp = queue->nexttick;	/* tempo di permanenza nella coda */

  /* aumento la dimensione del pacchetto per tenere conto dell' approssimazione */
  size += queue->time_exc;
  /* calcola il prossimo momento in cui togliere un pacchetto dalla coda */
  queue->nexttick += size / bytes_per_tick;
  /* calcola l'approssimazione del calcolo precedente */
  queue->time_exc = size % bytes_per_tick;
  /* aumenta la dimensione occupata dalla coda */
  queue->rule->size += (skb->tail - skb->head);
    
  queue->rule->payload += (skb->tail - skb->data);
  /* se ci sono già elementi nella coda aggiunge il pacchetto in fondo */
  if (queue->head)
    {
      queue->tail->next = pk;
      queue->tail = pk;
    }
  else
    {
      /* altrimenti viene messo in testa */
      queue->head = queue->tail = pk;
      /* programma il kernel timer per deaccodare il pacchetto al momento opportuno */
      queue->timer.expires = pk->timestamp;
      /* accende il flag del timer */
      timerflag++;
    }
  if (timerflag)
    /* fa partire il timer */
    add_timer (&queue->timer);
  /* valore di ritorno all' hook per mettere il pacchetto in sospeso */
  return NF_STOLEN;
}

/* funzinoe per togliere un pacchetto da una coda */

void
netshaper_dequeue_packet (unsigned long data)
{
  struct netshaper_queue *q = (struct netshaper_queue *) data;	/* la coda */
  struct netshaper_packet *p;	/* l'elemento della coda */
  unsigned long flags;		/* flag per il locking */

  /* mette un lock alla coda */
  spin_lock_irqsave (&q->lock, flags);

  /* butta i pachetti in timeout */
  while ((p = q->head) && (p->timestamp <= jiffies))
    {
      /* diminuisci lo spazio occupato dalla coda */
      q->rule->size -= ( p->skb->tail - p->skb->head );
      q->rule->payload -= ( p->skb->tail - p->skb->data );
      /* sposta la testa della coda */
      q->head = p->next;
      /* se la coda è finita metti a null il fondo */
      if (q->tail == p)
	q->tail = NULL;
      /* rilscia il lock sulla coda per eliminare il pacchetto */
      spin_unlock_irqrestore (&q->lock, flags);
      /* togli il paccheto */
      p->okfn (p->skb);
      /* dealloca lo spazion del netshaper_packet che conteneva il pacchetto */
      kfree (p);
#ifdef DEBUGG
      printk ("netshaper:Ho tolto dalla coda un pacchetto: causa timeout\n");
#endif
      /* riprendi il lock */
      spin_lock_irqsave (&q->lock, flags);
    }
  /* se ci sono ancora pacchetti riaggiorna il timer nella coda */
  if (p)
    {
      /* il prossimo istante in cui deallocare il pacchetto */
      q->timer.expires = p->timestamp;
      add_timer (&q->timer);	/* registra il timer */
    }
  /* rilascia il lock */
  spin_unlock_irqrestore (&q->lock, flags);
}

#ifndef NETSHAPER_LIMIT_KB	/* massima dimensione di una coda */
#  define NETSHAPER_LIMIT_KB 512	/* 512 kb per ogni coda */
#endif

/* calcola il timeout (in jiffy ) in funzione della massima dimensione della coda e alla*/
/* banda impostata nella regola */

int
netshaper_retimeout (int timeout, int bytes_per_tick)
{
  /* massimo numero di tick per cui rimane accodato */
  int maxticks = NETSHAPER_LIMIT_KB * 1024 / bytes_per_tick;
  /* se viene definito dall' utente un timeout maggiore usa quello */
  if (maxticks < timeout)
    timeout = maxticks;
  return timeout;
}

/* Ricerca una regola soddisfatta e ritorna un puntatore alla regola, se non la trova NULL */
struct netshaper_rule *
netshaper_find_rule (__u32 saddr,
		     __u32 daddr,
		     __u16 sport,
		     __u16 dport,
		     struct net_device *in,
		     struct net_device *out,
		     __u8 protocol_type, struct netshaper_rule ***prevp)
{
  /* regola d'appoggio e prima regola */
  struct netshaper_rule *rule, **prev = &first_rule;

#ifdef DEBUGG
  printk ("netshaper: Cerco una regola... ");
#endif

  /* ricerca della regola */
  while (*prev && (((*prev)->saddr != saddr) || ((*prev)->daddr != daddr) ||
		   ((*prev)->sport != sport) || ((*prev)->dport != dport) ||
		   ((*prev)->protocol_type != protocol_type)
		   || ((*prev)->in != in) || ((*prev)->out != out)))
    {
#ifdef DEBUGG
      printk ("netshaper: Rule proto: %d pacco proto: %d\n",
	      (*prev)->protocol_type, protocol_type);
#endif
      prev = &((*prev)->next);
    }
  /* non trovata ritorna NULL */
  if (!*prev)
    {
      *prevp = NULL;
#ifdef DEBUGG
      printk ("netshaper: non trovata \n");
#endif
      return NULL;
    }
  rule = *prev;
  *prevp = prev;
#ifdef DEBUGG
  printk ("netshaper: trovata \n");
  printk
    ("netshaper: find: regola: s: %d.%d.%d.%d:%d -- d:%d.%d.%d.%d:%d proto: %d\n",
     NIPQUAD ((*prev)->saddr), (*prev)->sport, NIPQUAD ((*prev)->daddr),
     (*prev)->dport, (*prev)->protocol_type);
  printk
    ("netshaper: find: regola1: s: %d.%d.%d.%d:%d -- d:%d.%d.%d.%d:%d proto: %d\n",
     NIPQUAD (saddr), sport, NIPQUAD (daddr), dport, protocol_type);
#endif
  /* trovata: ritorna la regola */
  return rule;
}

/* funzione per agingere una regola */

int
netshaper_add_rule (__u32 saddr,	/* indirizzo sorgente */
		    __u32 daddr,	/* indirizzo destinazione */
		    __u16 sport,	/* porta sorgente */
		    __u16 dport,	/* porta destinazione */
		    struct net_device *in, struct net_device *out, u32 smask,	/* netmask sorgente */
		    u32 dmask,	/* netmask destinazione */
		    __u8 protocol_type,	/* tipo di protocollo */
		    int bytes_per_tick,	/* bytes al tick */
		    int max_time)	/* massimo tempo di pemanenza nella coda */
{
  /* regola d'appoggio */
  struct netshaper_rule *rule;
#ifdef DEBUGG
  printk ("netshaper: aggiungo una regola\n ");
#endif
  /* alloca lo spazio per contenere la regola */
  rule = kmalloc (sizeof (struct netshaper_rule), GFP_KERNEL);
  if (!rule)
    return -ENOMEM;

  /* inizializza la regola */
  rule->saddr = saddr;
  rule->daddr = daddr;
  rule->in = in;
  rule->out = out;
  rule->smask = smask;
  rule->dmask = dmask;
  rule->protocol_type = protocol_type;
  rule->sport = sport;
  rule->dport = dport;
  rule->bytes_per_tick = bytes_per_tick;
  rule->max_time = max_time;
  rule->size = 0;
  rule->payload = 0;
  /* alloca lo spazio per contenere le code di input e output */
  rule->out_queue = kmalloc (sizeof (struct netshaper_queue), GFP_KERNEL);
  rule->in_queue = kmalloc (sizeof (struct netshaper_queue), GFP_KERNEL);

  if (!rule->out_queue || !rule->in_queue)
    {
      if (rule->out_queue)
	kfree (rule->out_queue);
      if (rule->in_queue)
	kfree (rule->in_queue);
      kfree (rule);
      return -ENOMEM;
    }

  /* pulisce la memoria in cui c'è la coda di input */
  memset (rule->in_queue, 0, sizeof (struct netshaper_queue));
  /* start del timer in input */
  init_timer (&rule->in_queue->timer);
  /* l'ggetto su cui lavora la timer function */
  rule->in_queue->timer.data = (unsigned long) rule->in_queue;
  /* assegna la timer function (la funzione per deaccodare un pacchetto) */
  rule->in_queue->timer.function = netshaper_dequeue_packet;

  rule->in_queue->rule = rule;
  
  /* inzializza lo spinlock della coda di input */
  spin_lock_init (&rule->in_queue->lock);

  memset (rule->out_queue, 0, sizeof (struct netshaper_queue));
  init_timer (&rule->out_queue->timer);

  rule->out_queue->timer.data = (unsigned long) rule->out_queue;
  rule->out_queue->timer.function = netshaper_dequeue_packet;

  rule->out_queue->rule = rule;
  
  spin_lock_init (&rule->out_queue->lock);

  /* la prima regola diventa la testa della pila di regole */
  rule->next = first_rule;
  /* la regola nuova diventa la prima regola */
  first_rule = rule;
  return 0;
}

/* funzione per rimuvere una regola */
void
netshaper_kfree_rule (struct netshaper_rule *rule)
{

  struct netshaper_packet *packet, *nextp;

#ifdef DEBUGG
  printk ("netshaper: elimino le regole\n ");
#endif

  /* cancella i timer delle code della regola */
  


#ifdef BASTARD
  del_timer (&rule->out_queue->timer);
  del_timer (&rule->in_queue->timer); 
#else
  del_timer_sync (&rule->out_queue->timer);
  del_timer_sync (&rule->in_queue->timer); 
#endif
 
  /* toglie tutti i pacchetti dalla coda di output */
  for (packet = rule->out_queue->head; packet;)
    {
      nextp = packet->next;
      packet->okfn (packet->skb);
      kfree (packet);
      packet = nextp;
    }
  /* libera la memora occupata dalla coda di out */
  kfree (rule->out_queue);

  for (packet = rule->in_queue->head; packet;)
    {
      nextp = packet->next;
      packet->okfn (packet->skb);
      kfree (packet);
      packet = nextp;
    }
  kfree (rule->in_queue);

  /* libera la memoria occupata dalla regola */
  kfree (rule);
}

/* funzione per valutare se un pacchetto soddisfa una regola */
unsigned int
netshaper_test_packet (struct sk_buff **skb_p,
		       int (*okfn) (struct sk_buff *),
		       const struct net_device *in,
		       const struct net_device *out,
		       unsigned int queue_position)
{
/* parametri nell'ordine: */
/* il pacchetto vero */
/* la funzione per togliere il apcchetto */
/* identifica in quale coda mettere il pacchetto */

  struct netshaper_rule *rule;	/* regola di appoggio */
  struct netshaper_queue *queue =  NULL;	/* coda in cui mettere i pacchetti */
  int retval = NF_ACCEPT;	/* valore di default da ritornare all' hook */

  unsigned long flags;		/* flags per il locking */
  
  struct sk_buff *skb = (*skb_p);	/* puntatore al pacchetto vero */
  struct iphdr *iph = NULL;	/* header IP */
  struct tcphdr *tcph = NULL;	/* header TCP */
  struct udphdr *udph = NULL;	/* header UDP */
  __u32 saddr;			/* indirizzo sorgente */
  __u32 daddr;			/* indirizzo destinatario */
  __u8 protocol_type;		/* tipo di protocollo */
  __u16 dport = 0;		/* porta destinazione */
  __u16 sport = 0;		/* porta sorgente */

  /* v. skbuff.h */
  iph = skb->nh.iph;		/* inizializza header IP */
  saddr = iph->saddr;		/* inizializza indizzo sorgente */
  daddr = iph->daddr;		/* inizializza indizzo destinazione */
  protocol_type = iph->protocol;	/* inizializza il protocollo */


  if (protocol_type == IPPROTO_TCP)
    {				/* se proto = TCP */
      if ((skb->h.th) != NULL)
	{
	  tcph = (struct tcphdr *) ((__u32 *) iph + iph->ihl);	/*inzializza TCP header */
	  if ((tcph->dest))
	    {
	      dport = tcph->dest;	/* inizializza porta destinazine */
	    }
	  else
	    {
	      printk ("TCP D port NULL!!\n");
	    }

	  if ((tcph->source))
	    {
	      sport = tcph->source;	/* inizializza porta sorgente */
	    }
	  else
	    {
	      printk ("TCP S port NULL!!\n");
	    }
	}
      else
	{
	  printk ("TCP HEADER NULL!!\n");
	}
    }

  if (protocol_type == IPPROTO_UDP)
    {				/* se proto = UDP */
      if ((skb->h.uh) != NULL)
	{
	  udph = (struct udphdr *) ((__u32 *) iph + iph->ihl);
	  if ((udph->dest))
	    {
	      dport = udph->dest;
	    }
	  else
	    {
	      printk ("UDP D port NULL!!\n");
	    }

	  if ((udph->source))
	    {
	      sport = udph->source;
	    }
	  else
	    {
	      printk ("UDP S port NULL!!\n");
	    }
	}
      else
	{
	  printk ("UDP HEADER NULL!!\n");
	}
    }

  /* ciclo per navigare nello stack delle regole */
  for (rule = first_rule; rule; rule = rule->next)
    {
      /* valuta in che coda inserire il pacchetto */
      switch (queue_position)
	{
	case IN_QUEUE:
	  queue = rule->in_queue;	// input
	  break;

	case OUT_QUEUE:
	  queue = rule->out_queue;	// output
	  break;
	}



#ifdef DEBUGG
      printk
	("netshaper: regola: %d.%d.%d.%d:%d --- %d.%d.%d.%d:%d proto:%d \n",
	 NIPQUAD (rule->saddr), rule->sport, NIPQUAD (rule->daddr),
	 rule->dport, rule->protocol_type);
      printk
	("netshaper: pacco: %d.%d.%d.%d:%d --- %d.%d.%d.%d:%d proto:%d \n",
	 NIPQUAD (saddr), sport, NIPQUAD (daddr), dport, protocol_type);
#endif

      /* protocollo */
      if (rule->protocol_type != 0)
	{
	  if (rule->protocol_type != protocol_type)
	    goto next;
#ifdef DEBUGG
	  printk ("netshaper: proto uguale: %d\n", protocol_type);
#endif
	}

      if (rule->in != NULL)
	{
#ifdef DEBUGG
	  printk ("netshaper: regola in iface %s\n", rule->in->name);
#endif
	  if (in == NULL)
	    goto next;
	  if (rule->in != in)
	    goto next;
	}

      if (rule->out != NULL)
	{
#ifdef DEBUGG
	  printk ("netshaper: regola out iface %s\n", rule->out->name);
#endif /* ; */
	  if (out == NULL)
	    goto next;
	  if (rule->out != out)
	    goto next;
	}


      /* se nella regola sono definiti entrabi gli indirizzi */
      if ((rule->daddr) && (rule->saddr))
	{
#ifdef DEBUGG
	  printk (" netshaper: 1 definito daddr e saddr\n");
#endif
	  /* se nella regola sono definite entrambe le porte */
	  if ((rule->dport != 0) && (rule->sport != 0))
	    {
#ifdef DEBUGG
	      printk ("netshaper: 2 definito dport e sport\n");
#endif
	      /* se il pacchetto ha le stesse porte e indirizzi della regola */
	      if (((rule->saddr & rule->smask) == (saddr & rule->smask)) &&	//indirizzo s
		  ((rule->daddr & rule->dmask) == (daddr & rule->dmask)) &&	//indirizzo d
		  (rule->dport == dport) && (rule->sport == sport))
		{		// le porte
#ifdef DEBUGG
		  printk ("netshaper: accodo un paccehtto\n");
#endif
		  /* se il pacchetto ci sta nella coda accodalo */
		  if (queue->nexttick <= jiffies + rule->max_time)
		    {
		      /* prende il lock della coda */
		      spin_lock_irqsave (&queue->lock, flags);
		      /* accoda */
		      retval = netshaper_enqueue_packet (skb,
							 queue,
							 rule->bytes_per_tick,
							 okfn);
                      
		      /* rilascia il lock */
		      spin_unlock_irqrestore (&queue->lock, flags);
		    }
		  else
		    {
		      /* se il pacchetto non ci sta nella coda viene scartato */
#ifdef DEBUGG
		      printk
			("netshaper: Coda troppo lunga scarto il pacchetto\n");
#endif
		      retval = NF_DROP;
		    }
		  return retval;
		}
	      goto next;
	    }
	  /* se nella regola è definita almeno una porta */
	  if ((rule->dport != 0) || (rule->sport != 0))
	    {
#ifdef DEBUGG
	      printk ("netshaper:3 definito dport o sport\n");
#endif
	      if (((rule->saddr & rule->smask) == (saddr & rule->smask)) &&	//indirizzo s
		  ((rule->daddr & rule->dmask) == (daddr & rule->dmask)) &&	//indirizzo d
		  ((rule->dport == dport) || (rule->sport == sport)))
		{		//una porta
#ifdef DEBUGG
		  printk ("netshaper: accodo un paccehtto\n");
#endif
		  if (queue->nexttick <= jiffies + rule->max_time)
		    {
		      spin_lock_irqsave (&queue->lock, flags);
		      retval = netshaper_enqueue_packet (skb,
							 queue,
							 rule->bytes_per_tick,
							 okfn);
                  
		      spin_unlock_irqrestore (&queue->lock, flags);
		    }
		  else
		    {
#ifdef DEBUGG
		      printk
			("netshaper: Coda troppo lunga scarto il pacchetto\n");
#endif
		      retval = NF_DROP;
		    }
		  return retval;
		}
	      goto next;
	    }
	  goto next;
	}
      /* se nella regola è definito un indirizzo */
      if ((rule->daddr) || (rule->saddr))
	{
#ifdef DEBUGG
	  printk ("netshaper:4 definito daddr o saddr\n");
#endif
	  /* se nella regola sono definite entrambe le porte */
	  if ((rule->dport != 0) && (rule->sport != 0))
	    {
#ifdef DEBUGG
	      printk ("netshaper: 5 definito dport e sport\n");
#endif
	      if (((rule->saddr & rule->smask) == (saddr & rule->smask)) ||	//indirizzo s
		  (((rule->daddr & rule->dmask) == (daddr & rule->dmask)) &&	// O indirizzo d
		   (rule->dport == dport) && (rule->sport == sport)))
		{		//entrambe le porte
#ifdef DEBUGG
		  printk ("netshaper: accodo un paccehtto\n");
#endif
		  if (queue->nexttick <= jiffies + rule->max_time)
		    {
		      spin_lock_irqsave (&queue->lock, flags);
		      retval = netshaper_enqueue_packet (skb,
							 queue,
							 rule->bytes_per_tick,
							 okfn);
                    
		      spin_unlock_irqrestore (&queue->lock, flags);
		    }
		  else
		    {
#ifdef DEBUGG
		      printk
			("netshaper: Coda troppo lunga scarto il pacchetto\n");
#endif
		      retval = NF_DROP;
		    }
		  return retval;
		}
	      goto next;
	    }
	  /* se nella regola è definita almeno una porta */
	  if ((rule->dport != 0) || (rule->sport != 0))
	    {
#ifdef DEBUGG
	      printk ("netshaper: 6 definito dport o sport\n");
#endif
	      if ((((rule->saddr & rule->smask) == (saddr & rule->smask)) ||	//indirizzo s
		   ((rule->daddr & rule->dmask) == (daddr & rule->dmask))) &&	//O indirizzo d
		  ((rule->dport == dport) || (rule->sport == sport)))
		{		//una porta
#ifdef DEBUGG
		  printk ("netshaper: accodo un paccehtto\n");
#endif
		  if (queue->nexttick <= jiffies + rule->max_time)
		    {
		      spin_lock_irqsave (&queue->lock, flags);
		      retval = netshaper_enqueue_packet (skb,
							 queue,
							 rule->bytes_per_tick,
							 okfn);
                    
		      spin_unlock_irqrestore (&queue->lock, flags);
		    }
		  else
		    {
#ifdef DEBUGG
		      printk
			("netshaper: Coda troppo lunga scarto il pacchetto\n");
#endif
		      retval = NF_DROP;
		    }
		  return retval;
		}
	      goto next;
	    }
	}
      /* se nella regola sono definiti entrambi gli indirizzi e non le porte */
      if ((((rule->daddr) && (rule->saddr))) &&
	  ((!rule->dport) && (!rule->sport)))
	{
#ifdef DEBUGG
	  printk ("netshaper: 7 definito daddr o saddr\n");
#endif
	  if (((rule->saddr & rule->smask) == (saddr & rule->smask)) &&	//indirizzo s
	      ((rule->daddr & rule->dmask) == (daddr & rule->dmask)))
	    {			// indirizzo d
#ifdef DEBUGG
	      printk ("netshaper: accodo un paccehtto\n");
#endif
	      if (queue->nexttick <= jiffies + rule->max_time)
		{
		  spin_lock_irqsave (&queue->lock, flags);
		  retval = netshaper_enqueue_packet (skb,
						     queue,
						     rule->bytes_per_tick,
						     okfn);
                  
		  spin_unlock_irqrestore (&queue->lock, flags);
		}
	      else
		{
#ifdef DEBUGG
		  printk
		    ("netshaper: Coda troppo lunga scarto il pacchetto\n");
#endif
		  retval = NF_DROP;
		}
	      return retval;
	    }
	  goto next;
	}

      /* se nella regola è definito un indirizzo e non le porte */
      if ((((rule->daddr) || (rule->saddr))) &&
	  ((!rule->dport) && (!rule->sport)))
	{
#ifdef DEBUGG
	  printk ("netshaper: 8 definito daddr o saddr\n");
#endif
	  if (((rule->saddr & rule->smask) == (saddr & rule->smask)) ||	//indirizzo s
	      ((rule->daddr & rule->dmask) == (daddr & rule->dmask)))
	    {			// O indirizzo d
#ifdef DEBUGG
	      printk ("netshaper: accodo un paccehtto\n");
#endif
	      if (queue->nexttick <= jiffies + rule->max_time)
		{
		  spin_lock_irqsave (&queue->lock, flags);
		  retval = netshaper_enqueue_packet (skb,
						     queue,
						     rule->bytes_per_tick,
						     okfn);
                  
		  spin_unlock_irqrestore (&queue->lock, flags);
		}
	      else
		{
#ifdef DEBUGG
		  printk
		    ("netshaper: Coda troppo lunga scarto il pacchetto\n");
#endif
		  retval = NF_DROP;
		}
	      return retval;
	    }
	  goto next;
	}

      /*se nella regola sono definite solo le due porte e non gli indirizzi */
      if (((rule->dport != 0) && (rule->sport != 0)) &&
	  ((!rule->daddr) && (!rule->saddr)))
	{
#ifdef DEBUGG
	  printk ("netshaper: 9 definito dport e sport\n");
#endif
	  if ((rule->sport == sport) &&	//porta s
	      (rule->dport == dport))
	    {			//porta d
#ifdef DEBUGG
	      printk ("netshaper: accodo un paccehtto\n");
#endif
	      if (queue->nexttick <= jiffies + rule->max_time)
		{
		  spin_lock_irqsave (&queue->lock, flags);
		  retval = netshaper_enqueue_packet (skb,
						     queue,
						     rule->bytes_per_tick,
						     okfn);
                  
		  spin_unlock_irqrestore (&queue->lock, flags);
		}
	      else
		{
#ifdef DEBUGG
		  printk
		    ("netshaper: Coda troppo lunga scarto il pacchetto\n");
#endif
		  retval = NF_DROP;
		}
	      return retval;
	    }
	  goto next;
	}

      /* se nella regola compare solo una porta */
      if (((rule->dport != 0) || (rule->sport != 0)) &&
	  ((!rule->daddr) && (!rule->saddr)))
	{
#ifdef DEBUGG
	  printk ("netshaper: 10 definito dport o sport\n");
#endif
	  if ((rule->sport == sport) ||	//porta s
	      (rule->dport == dport))
	    {			// O porta d
#ifdef DEBUGG
	      printk ("netshaper: accodo un paccehtto\n");
#endif
	      if (queue->nexttick <= jiffies + rule->max_time)
		{
		  spin_lock_irqsave (&queue->lock, flags);
		  retval = netshaper_enqueue_packet (skb,
						     queue,
						     rule->bytes_per_tick,
						     okfn);
                  
		  spin_unlock_irqrestore (&queue->lock, flags);
		}
	      else
		{
#ifdef DEBUGG
		  printk
		    ("netshaper: Coda troppo lunga scarto il pacchetto\n");
#endif
		  retval = NF_DROP;
		}
	      return retval;
	    }
	  goto next;
	}
      /* se nella regola non è definito niente */
      if (((rule->dport == 0) && (rule->sport == 0)) &&
	  ((!rule->daddr) && (!rule->saddr)))
	{
#ifdef DEBUGG
	  printk ("netshaper: 11 definito niente\n");
#endif
#ifdef DEBUGG
	  printk ("netshaper: accodo un paccehtto\n");
#endif
	  if (queue->nexttick <= jiffies + rule->max_time)
	    {
	      spin_lock_irqsave (&queue->lock, flags);
	      retval = netshaper_enqueue_packet (skb,
						 queue,
						 rule->bytes_per_tick, 
                                                 okfn);
              
	      spin_unlock_irqrestore (&queue->lock, flags);
	    }
	  else
	    {
#ifdef DEBUGG
	      printk ("netshaper: Coda troppo lunga scarto il pacchetto\n");
#endif
	      retval = NF_DROP;
	    }
	  return retval;
	}
      goto next;
    next:;
    }
  return retval;
}

/* funzioni chiamte quando un pacchetto passa in un hook */

/* prerouting hook function */
unsigned int
netshaper_hook_in (unsigned int hooknum,	/* proiorità */
		   struct sk_buff **skb_p,	/* pacchetto */
		   const struct net_device *in,	/* dev di ingresso */
		   const struct net_device *out,	/* dev di uscita */
		   int (*okfn) (struct sk_buff *))	/* funzione per eliminare il pacchetto */
{
  /* controlla se il pacchetto soddisfa una regola */
#ifdef DEBUGG
  printk ("netshaper: IN: nuovo pacchetto\n");
  printk ("netshaper: pacco in iface %s\n", in->name);
  printk ("netshaper: pacco out iface %s\n", out->name);
#endif
  return netshaper_test_packet (skb_p, okfn, in, out, IN_QUEUE);
}

/* postrouting hook function */
unsigned int
netshaper_hook_out (unsigned int hooknum,
		    struct sk_buff **skb_p,
		    const struct net_device *in,
		    const struct net_device *out,
		    int (*okfn) (struct sk_buff *))
{
#ifdef DEBUGG
  printk ("netshaper: OUT: nuovo pacchetto\n");
  printk ("netshaper: pacco in iface %s\n", in->name);
  printk ("netshaper: pacco out iface %s\n", out->name);
#endif
  return netshaper_test_packet (skb_p, okfn, in, out, OUT_QUEUE);
}

/* Inizializzazione degli hook */

/* postrouting hook */
struct nf_hook_ops netshaper_hook_out_ops = {
  hook:netshaper_hook_out,	/* funzionde da chiamare */
  pf:PF_INET,			/* tipo di pacchetti (ipv4) */
  hooknum:NF_IP_POST_ROUTING,	/* posizione dell'hook */
};

/* prerouting hook */
struct nf_hook_ops netshaper_hook_in_ops = {	/*OUTPUT hook */
  hook:netshaper_hook_in,
  pf:PF_INET,
  hooknum:NF_IP_PRE_ROUTING,
};

/* implemetazione della Input/Output control function */
int
netshaper_ioctl (struct net_device *dev, struct ifreq *ifr, int cmd)
{
  unsigned long flags;		/* flag per il locking */

  /* dati scritti dallo user-space (uptr è un user space) */
  struct netshaper_userinfo info, *uptr =
    (struct netshaper_userinfo *) ifr->ifr_data;

  struct netshaper_rule *rule, **prev;
  struct net_device *in = NULL;
  struct net_device *out = NULL;

  /* banda */
  int bw;

  /* se non sei root errore */
 
#ifndef BASTARD
  if (!suser ())
    {
      #ifdef DEBUGG
        printk ("Non hai i permessi\n");
      #endif
      return -EPERM;
    }
#endif

  /* verifica di potere leggere la memoria in cui ci sono i dati dallo user-space */
  if (!access_ok (VERIFY_READ, uptr, sizeof (*uptr)))
    {
      #ifdef DEBUGG
        printk ("Non puoi leggere la memoria\n");
      #endif
      return -EFAULT;
    }

  /* copia i dati dalla memoria in user-space a quella in kernel-space  */
  copy_from_user (&info, uptr, sizeof (info));

  /* valuta il comando ricevuto dallo user-space */
  switch (cmd)
    {
    case SIOCNETSHAPERSET:	/* modifica le regole */

#ifdef DEBUGG
      printk
	("ioctl: S: %d.%d.%d.%d, S: %d.%d.%d.%d, sm:%d.%d.%d.%d dm:%d.%d.%d.%d banda:%i, to:%i ms proto: %d\n",
	 NIPQUAD (info.saddr), NIPQUAD (info.daddr), NIPQUAD (info.smask),
	 NIPQUAD (info.dmask), (int) info.bytes_per_second, info.max_time,
	 info.protocol_type);
#endif

      /* check della correttezza (HZ = 100) */
      if (info.bytes_per_second && info.bytes_per_second < HZ)
        {
          #ifdef DEBUGG
            printk ("Banda non corretta\n");
          #endif      
          return -EINVAL;
        }
      /* trasforma la banda in bytes per tick */
      //bw = (info.bytes_per_second + HZ / 2) / HZ;
      bw = (info.bytes_per_second) / HZ;
      if (info.max_time < 0 || info.max_time > 30000)
        {
          #ifdef DEBUGG
            printk ("timeout esagerato\n");
          #endif
          return -EINVAL;
        }
      /* max_time in kernel space in jiffies */
      info.max_time = info.max_time * HZ / 1000;
      /* cerca se esiste già una regola */
      if (info.in != NULL)
	{
	  if ((in = dev_get_by_name (info.in)) == NULL)
	    {
	      printk ("netshaper: %s non esiste\n", info.in);
	      return -EINVAL;
	    }
	  dev_put (in);
	}

      if (info.out != NULL)
	{
	  if ((out = dev_get_by_name (info.out)) == NULL)
	    {
	      printk ("netshaper: %s non esiste\n", info.out);
	      return -EINVAL;
	    }
	  dev_put (out);
	}

      /* prede il lock sulla regola */
      spin_lock_irqsave (&netshaper_rule_lock, flags);

      rule = netshaper_find_rule (info.saddr,
				  info.daddr,
				  info.sport,
				  info.dport,
				  in, out, info.protocol_type, &prev);
      if (rule)
	{			/* se esiste la modifica */
	  if (info.bytes_per_second)
	    {			/* se è settata una banda sovrascrive la regola */
	      rule->bytes_per_tick = bw;
	      rule->max_time = netshaper_retimeout (info.max_time, bw);
	      rule->smask = info.smask;
	      rule->dmask = info.dmask;
	      rule->dport = info.dport;
	      rule->sport = info.sport;
	      rule->in = in;
	      rule->out = out;
	      rule->protocol_type = info.protocol_type;
	    }
	  else
	    {			/* se non c'è banda elimina la regola */
	      *prev = rule->next;
	      netshaper_kfree_rule (rule);
	    }
	}
      /* se non c'è la regola la crea */
      else if (info.bytes_per_second)
	{
	  netshaper_add_rule (info.saddr,
			      info.daddr,
			      info.sport,
			      info.dport,
			      in,
			      out,
			      info.smask,
			      info.dmask,
			      info.protocol_type,
			      bw, netshaper_retimeout (info.max_time, bw));
	}
      /*restituisce il lock */
      spin_unlock_irqrestore (&netshaper_rule_lock, flags);
      return 0;


    default:
        #ifdef DEBUGG
          printk ("comando ioctl sbagliato!\n");
        #endif
      return -ENOTTY;
    }
  return 0;
}

/* inizializza il device */
int
netshaper_init_dev (struct net_device *dev)
{
  dev->do_ioctl = netshaper_ioctl;	/* I/O control function */
  return 0;
}

/*device su cui scrivere le regole*/
struct net_device netshaper_dev = {
  name:"netshaper",		/* nome */
  init:netshaper_init_dev,	/* funzione di inizializzazione */
};

static int init_status_flag;	/* flag di stato */
#define NETSHAPER_IN_HOOK_REGISTERED		0x02	/* prerouting hook registred */
#define NETSHAPER_OUT_HOOK_REGISTERED		0x04	/* postrouting hook registred */
#define NETSHAPER_NETDEVICE_REGISTERED		0x06	/* device registred */
#define NETSHAPER_PROC_DIR_ENTRY_REGISTERED	0x08	/* /proc dir entry registrate */
#define NETSHAPER_PROC_FILE_ENTRY_REGISTERED	0x20	/* /proc file entry registrate */

/* funzione per rimuvere il modulo */
void
cleanup_module (void)
{
  struct netshaper_rule *rule, *next;
  unsigned long flags;

  /* se registrato toglie il prerouting hook */
  if (init_status_flag & NETSHAPER_IN_HOOK_REGISTERED)
    nf_unregister_hook (&netshaper_hook_in_ops);

  /* se registrato toglie il postrouting hook */
  if (init_status_flag & NETSHAPER_OUT_HOOK_REGISTERED)
    nf_unregister_hook (&netshaper_hook_out_ops);

  /* lock sulle regole */
  spin_lock_irqsave (&netshaper_rule_lock, flags);
  /* elimina tutte le regole */
  while ((rule = first_rule))
    {				/* free it all */
      first_rule = rule->next;
      netshaper_kfree_rule (rule);
      rule = next;
    }
  /* restituisce il lock */
  spin_unlock_irqrestore (&netshaper_rule_lock, flags);

  /* se registrato toglie il device */
  if (init_status_flag & NETSHAPER_NETDEVICE_REGISTERED)
    unregister_netdev (&netshaper_dev);

  /* rimuve il /proc file */
  if (init_status_flag & NETSHAPER_PROC_FILE_ENTRY_REGISTERED)
    remove_proc_entry (NETSHAPER_PROC_FILE, netshaper_proc_dir);

  /* rimuve la /proc dir */
  if (init_status_flag & NETSHAPER_PROC_DIR_ENTRY_REGISTERED)
    remove_proc_entry (NETSHAPE_PROC_DIR, NULL);
}

/* funzione di inzializzazione del modulo */
int
init_module (void)
{
  int res;
  /* inizializza lo spinlock */
  spin_lock_init (&netshaper_rule_lock);
  /* registra il prerouting hook */
  res = nf_register_hook (&netshaper_hook_in_ops);
  if (res < 0)
    {
      printk ("netshaper: non posso registrare il prerouting hook\n");
      cleanup_module ();
      return res;
    }
  init_status_flag |= NETSHAPER_IN_HOOK_REGISTERED;

  /* registra il postrouting hook */
  res = nf_register_hook (&netshaper_hook_out_ops);
  if (res < 0)
    {
      printk ("netshaper: non posso registrare il postrouting hook\n");
      cleanup_module ();
      return res;
    }
  init_status_flag |= NETSHAPER_OUT_HOOK_REGISTERED;

  /* registra il device */
  res = register_netdev (&netshaper_dev);
  if (res)
    {
      printk ("netshaper: can't register netdevice\n");
      cleanup_module ();
      return res;
    }
  init_status_flag |= NETSHAPER_NETDEVICE_REGISTERED;

  /* crea la dir in /proc */
  netshaper_proc_dir = proc_mkdir (NETSHAPE_PROC_DIR, NULL);
  if (netshaper_proc_dir == NULL)
    {
      printk ("netshaper: can't register proc dir entry\n");
      cleanup_module ();
      return -ENOMEM;
    }
  netshaper_proc_dir->owner = THIS_MODULE;

  init_status_flag |= NETSHAPER_PROC_DIR_ENTRY_REGISTERED;    
  
  /* crea il file in /proc/netshaper */
  netshaper_proc_file = create_proc_read_entry (NETSHAPER_PROC_FILE, 0444,
						netshaper_proc_dir,
						netshaper_proc_read_file,
						NULL);
  if (netshaper_proc_file == NULL)
    {
      printk ("netshaper: can't register proc file entry\n");
      cleanup_module ();
      return -ENOMEM;
    }
  netshaper_proc_file->owner = THIS_MODULE;
  init_status_flag |= NETSHAPER_PROC_FILE_ENTRY_REGISTERED;
  return 0;
}
