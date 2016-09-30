RELEASE=0.85
#DEBUG = y

ifndef KERNELDIR
KERNELDIR =  /usr/src/linux-2.4.20/
endif

ifndef INCLUDEDIR
INCLUDEDIR = $(KERNELDIR)/include
endif

ifeq ($(DEBUG),y)
  DEBFLAGS = -Wall -O -g -DDEBUGG
else
  DEBFLAGS = -O2
endif

CFLAGS = -Wall $(DEBFLAGS) 

CFLAGS += -I$(INCLUDEDIR)

OBJS = netshaper_mod.o
CLIENTS = netshaper
MANPAGES = netshaper.8

VERSIONFILE = $(INCLUDEDIR)/linux/version.h
VERSION     = $(shell awk -F\" '/REL/ {print $$2}' $(VERSIONFILE))
INSTALLMOD = /lib/modules/$(VERSION)/misc
INSTALLMAN = /usr/share/man/man8/ 
INSTALLBIN = /usr/local/sbin


all: $(OBJS) $(CLIENTS)

install: all
	install -d $(INSTALLMOD) $(INSTALLMAN) $(INSTALLBIN)
	install -c $(OBJS)     $(INSTALLMOD)
	install -c $(CLIENTS)  $(INSTALLBIN)
	install -c $(MANPAGES) $(INSTALLMAN)

uninstall:
	for n in $(OBJS);     do rm -f $(INSTALLMOD)/$$n; done
	for n in $(CLIENTS);  do rm -f $(INSTALLBIN)/$$n; done
	for n in $(MANPAGES); do rm -f $(INSTALLMAN)/$$n; done

clean:
	rm -f *.o $(CLIENTS) *~ core

tar:
	@if [ "x" = "x$(RELEASE)" ]; then \
	    n=`basename \`pwd\``; cd ..; tar cvf - $$n | gzip > $$n.tar.gz; \
	    echo 'you can set a numeric $$(RELEASE) to make a named tar'; \
	else \
	    mkdir ../netshaper-$(RELEASE) || exit 1; \
	     cp -a . ../netshaper-$(RELEASE) && cd .. && \
	        tar --exclude '*/CVS*' \
			-cvzf netshaper-$(RELEASE).tar.gz netshaper-$(RELEASE); \
	fi

distrib: clean tar
