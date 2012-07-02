#
# Modules and the files they are made from
#
RBDDRIVER = rbd
MODULES = $(RBDDRIVER) 
CFILES = $(MODULES:=.c)
OBJFILES = $(CFILES:.c=.o)
KOFILES = $(OBJFILES:.o=.ko)

obj-m += $(OBJFILES)

# Make sure to set up dependencies between source and object files
%.o: %.c
%.ko: %.o

obj-m += rbd.o

KVERSION = $(shell uname -r)
all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean

#
# Convience targets
#

install: $(RBDDRIVER).ko
	insmod $(RBDDRIVER).ko

deinstall:
	rmmod $(KOFILES)

reinstall: deinstall install

.PHONY: install deinstall reinstall clean
