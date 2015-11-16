#
# frontier client http Makefile
# 
# Author: Sergey Kosyakov
#
# $Id$
#
# Copyright (c) 2009, FERMI NATIONAL ACCELERATOR LABORATORY
# All rights reserved. 
#
# For details of the Fermitools (BSD) license see Fermilab-2009.txt or
#  http://fermitools.fnal.gov/about/terms.html
#

OBJ	=	fn-htclient.o fn-socket.o fn-urlparse.o
HDR	=	fn-htclient.h ../include/frontier_client/frontier.h ../fn-internal.h
INC	=	$(COMMON_INC) -I. -I.. -I../include

all: $(OBJ)
	rm -rf .libs
	mkdir .libs
	cp $(OBJ) .libs/

fn-htclient.o: fn-htclient.c $(HDR)
	$(CC) $(COPT) $(INC) -c fn-htclient.c

fn-socket.o: fn-socket.c $(HDR)
	$(CC) $(COPT) $(INC) -c fn-socket.c

fn-urlparse.o: fn-urlparse.c $(HDR)
	$(CC) $(COPT) $(INC) -c fn-urlparse.c


test-htclient: $(OBJ) test-htclient.c $(HDR)
	$(CC) $(COPT) $(INC) -o test-htclient test-htclient.c *.o

clean:
	rm -f *.o
	rm -f test-htclient
	rm -rf .libs

