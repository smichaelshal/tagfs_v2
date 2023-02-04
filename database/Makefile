SUBDIRS := db_fs

clean:
	for i in $(SUBDIRS); do $(MAKE) -C $$i clean; done
	rm -rf *.o *~ core .depend .*.cmd *.ko *.mod.c .tmp_versions *.mod modules.order *.symvers .*.o.d .*.o.cmd