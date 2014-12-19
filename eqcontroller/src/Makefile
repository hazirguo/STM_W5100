#
# Top level Makefile for the EQ controller package
#
EQCSYS	=	eqcsys
EQCGRP	=	equip
PREFIX	=	/usr/local/eqcontroller


all::
	make -C daemon all
	make -C driver all

install::	bootstrap
	make -C daemon install
	make -C driver install

clean::
	make -C daemon clean
	make -C driver clean

bootstrap::
	@if [ ! -d ${PREFIX} ]; then \
		mkdir ${PREFIX}; \
	fi
	@if [ ! -d ${PREFIX}/etc ]; then \
		mkdir ${PREFIX}/etc; \
		chown ${EQCSYS}:${EQCGRP} ${PREFIX}/etc; \
		chmod 775 ${PREFIX}/etc; \
	fi
	cp -p bootstrap/eqc.conf.default ${PREFIX}/etc/eqc.conf
	@if [ ! -d ${PREFIX}/include ]; then \
		mkdir ${PREFIX}/include; \
		chown ${EQCSYS}:${EQCGRP} ${PREFIX}/include; \
		chmod 775 ${PREFIX}/etc; \
	fi
	cp -p include/labeqc.h ${PREFIX}/include/labeqc.h
	@if [ ! -d ${PREFIX}/bin ]; then \
		mkdir ${PREFIX}/bin; \
		chown ${EQCSYS}:${EQCGRP} ${PREFIX}/bin; \
		chmod 775 ${PREFIX}/bin; \
	fi
	@if [ ! -d ${PREFIX}/log ]; then \
		mkdir ${PREFIX}/log; \
		chown ${EQCSYS}:${EQCGRP} ${PREFIX}/log; \
		chmod 775 ${PREFIX}/log; \
	fi
	@if [ ! -d ${PREFIX}/test-scripts ]; then \
		mkdir ${PREFIX}/test-scripts; \
		chown ${EQCSYS}:${EQCGRP} ${PREFIX}/test-scripts; \
		chmod 775 ${PREFIX}/test-scripts; \
	fi
	cp bootstrap/labeqc.init /etc/rc.d/init.d/labeqc
	chkconfig --add labeqc
	chkconfig --levels 345 labeqc on
