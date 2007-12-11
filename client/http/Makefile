#
# frontier client http Makefile
# 
# Author: Sergey Kosyakov
#
# $Id$
#
#  Copyright (C) 2007  Fermilab
#
#  This program is free software: you can redistribute it and/or
#  modify it under the terms of the GNU Lesser General Public License
#  as published by the Free Software Foundation, either version 3 of
#  the License, or (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  Lesser General Public License for more details.
#
#  You should have received a copy of the GNU Lesser General Public
#  License along with this program.  If not, see
#  <http://www.gnu.org/licenses/>.
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

