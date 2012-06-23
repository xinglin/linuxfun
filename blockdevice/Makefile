#
# Modules and the files they are made from
#
BLOCKDRIVER = blockdriver
MODULES = $(BLOCKDRIVER) 
CFILES = $(MODULES:=.c)
OBJFILES = $(CFILES:.c=.o)
KOFILES = $(OBJFILES:.o=.ko)

obj-m += $(OBJFILES)

# Make sure to set up dependencies between source and object files
%.o: %.c
%.ko: %.o

obj-m += blockdriver.o

KVERSION = $(shell uname -r)
all:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) modules
clean:
	make -C /lib/modules/$(KVERSION)/build M=$(PWD) clean

#
# Convience targets
#

install: $(BLOCKDRIVER).ko
	insmod $(BLOCKDRIVER).ko

deinstall:
	rmmod $(KOFILES)

reinstall: deinstall install

.PHONY: install deinstall reinstall clean
