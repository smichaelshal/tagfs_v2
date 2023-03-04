ifneq ($(KERNELRELEASE),)
# kbuild part of makefile

obj-m := vtagfs.o
vtagfs-y := vtagfs_mod.o

vtagfs-y +=  include/ramfs/libfs.o include/ramfs/file-mmu.o include/ramfs/inode.o

vtagfs-y += publisher/publisher.o
vtagfs-y += publisher/iget_types.o

# vtagfs-y += layout/layout.o
# vtagfs-y += layout/hardlinking/hardlinking.o

vtagfs-y += api/device_api.o

vtagfs-y += hooks/hooks.o

vtagfs-y += utils/utils.o

vtagfs-y += database/database.o

vtagfs-y += database/db_fs/db_fs.o

ccflags-y := -I$(src)
ccflags-y := -I$(src)/include/ramfs
ccflags-y := -I$(src)/publisher
# ccflags-y := -I$(src)/layout
# ccflags-y := -I$(src)/layout/hardlinking
ccflags-y := -I$(src)/api
ccflags-y := -I$(src)/hooks
ccflags-y := -I$(src)/utils
ccflags-y := -I$(src)/database
ccflags-y := -I$(src)/db_fs

else
# normal makefile

KDIR ?= /lib/modules/`uname -r`/build

SUBDIRS := include
# SUBDIRS += layout
SUBDIRS += publisher
SUBDIRS += user
SUBDIRS += api
SUBDIRS += hooks
SUBDIRS += utils
SUBDIRS += database
# SUBDIRS += db_fs

.PHONY: all clean
default: mod

mod:
	$(MAKE) -C $(KDIR) M=$$PWD modules

users:
	$(MAKE) -C user

all: mod users

default: mod




clean:
	for i in $(SUBDIRS); do $(MAKE) -C $$i clean; done
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions *.mod modules.order *.symvers .*.o.d .*.o.cmd
endif