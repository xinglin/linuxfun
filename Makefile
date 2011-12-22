#
# Modules and the files they are made from
#
PROCINFO = procinfo
MODULES = $(PROCINFO) 
CFILES = $(MODULES:=.c)
OBJFILES = $(CFILES:.c=.o)
KOFILES = $(OBJFILES:.o=.ko)

obj-m += $(OBJFILES)

# Make sure to set up dependencies between source and object files
%.o: %.c
%.ko: %.o

KVERSION = $(shell uname -r)
all: $(CFILES)
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean

#
# Convience targets
#

# insmod can't be used to install multiple modules at once. So create 
# this to install all modules at once.
install: procinfo_install

procinfo_install: $(PROCINFO).ko
	insmod $(PROCINFO).ko


deinstall:
	rmmod $(KOFILES)

reinstall: deinstall install

.PHONY: install deinstall reinstall clean
