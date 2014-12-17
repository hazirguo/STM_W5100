all: 
	make -C testdir all
	make -C /lib/modules/`uname -r`/build SUBDIRS=`pwd` modules

install: all
	make -C testdir install
	install -d /lib/modules/misc
	install -c labeqc.ko /lib/modules/misc

clean:
	rm -fr *.ko *.ko.unsigned *.o .*.cmd *.mod.c *~ .tmp_versions core
	make -C testdir clean

obj-m := labeqc.o
