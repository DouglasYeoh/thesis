# Comment/uncomment the following line to disable/enable debugging
DEBUG = n

# Add your debugging flag (or not) to CFLAGS
# "-O" is needed to expand inlines
ifeq ($(DEBUG),y)
  DEBFLAGS = -O -g -DDEBUG 
else
  DEBFLAGS = -O2
endif

EXTRA_CFLAGS += $(DEBFLAGS)
#EXTRA_CFLAGS += -I$(LDDINC)

ifneq ($(KERNELRELEASE),)
# call from kernel build system

#scull-objs := main.o pipe.o access.o

	obj-m	:= ds18b20.o


else

	KERNELDIR :=/tiny_6410/linux-2.6.38/
	PWD       := $(shell pwd)

modules:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions

depend .depend dep:
	$(CC) $(CFLAGS) -M *.c > .depend


ifeq (.depend,$(wildcard .depend))
include .depend
endif

endif

