#############################################################################
#
# Makefile for librf24 examples on Raspberry Pi
#
# License: GPL (General Public License)
# Author:  gnulnulf <arco@appeltaart.mine.nu>
# Date:    2013/02/07 (version 1.0)
#
# Description:
# ------------
# use make all and make install to install the examples
# You can change the install directory by editing the prefix line
#
prefix := /usr/local


# The compiler and recommended compiler flags for the Raspberry Pi
CC=g++
CCFLAGS=-g -Ofast -mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s
#CCFLAGS=-mfpu=vfp -mfloat-abi=hard -march=armv6zk -mtune=arm1176jzf-s

all: nodeserver

nodeserver: sensornet.o config.o
	$(CC) -lrf24network -lrf24-bcm -lqpid-proton sensornet.o config.o -o nodeserver
	chmod +x nodeserver

sensornet.o: sensornet.cpp
	$(CC) ${CCFLAGS} -Wall -c $^

config.o: config.cpp
	$(CC) ${CCFLAGS} -Wall -c $^

clean:
	rm -rf *.o nodeserver

#install: all
#	test -d $(prefix) || mkdir $(prefix)
#	test -d $(prefix)/bin || mkdir $(prefix)/bin
#	for prog in $(PROGRAMS); do \
#	  install -m 0755 $$prog $(prefix)/bin; \
#	done

#.PHONY: install
