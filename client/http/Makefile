#
# FroNTier client API type 1
# 
# Author: Sergey Kosyakov
#
# $Header$
# 
# $Id$
#

OBJ	=	fn-htclient.o fn-socket.o fn-urlparse.o
HDR	=	fn-htclient.h ../include/frontier_client/frontier.h ../fn-internal.h
INC	=	-I. -I.. -I../include

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

