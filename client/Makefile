#
# FroNTier client API type 1
# 
# Author: Sergey Kosyakov
#
# $Header$
# 
# $Id$
#

CC	=	cc
CXX	=	c++
INC	=	-Iexpat/lib -I/usr/local/include -I.
LIBDIR	=	-L/usr/local/lib
LIBS	=	-lcurl -lz
COPT	=	-pthread -Wall -g -O2 -DFRONTIER_DEBUG
#COPT	=	-pthread -Wall -O
CXXOPT	=

# To disable namespace coment line below
CXXOPT	+=	-DFNTR_USE_NAMESPACE

HDR	=	frontier.h

all: libfrontier.a


libfrontier.a: expat/lib/xmltok.o fn-base64.o fn-md5.o frontier.o response.o payload.o rs-bin.o memdata.o error.o frontier-cpp.o
	rm -f libfrontier.a
	ar qcv libfrontier.a frontier.o response.o payload.o rs-bin.o memdata.o error.o expat/lib/xmlparse.o expat/lib/xmlrole.o expat/lib/xmltok.o fn-base64.o fn-md5.o frontier-cpp.o
	ranlib libfrontier.a

expat/lib/xmltok.o:
	(cd expat; /bin/sh configure --prefix=`pwd`/../local --disable-shared --without-pic && make buildlib)

fn-base64.o: fn-base64.c $(HDR)
	$(CC) $(COPT) $(INC) -c fn-base64.c

fn-md5.o: fn-md5.c $(HDR)
	$(CC) $(COPT) $(INC) -c fn-md5.c

frontier.o: frontier.c $(HDR)
	$(CC) $(COPT) $(INC) -c frontier.c

response.o: response.c $(HDR)
	$(CC) $(COPT) $(INC) -c response.c

payload.o: payload.c $(HDR)
	$(CC) $(COPT) $(INC) -c payload.c

rs-bin.o: rs-bin.c $(HDR)
	$(CC) $(COPT) $(INC) -c rs-bin.c

memdata.o: memdata.c $(HDR)
	$(CC) $(COPT) $(INC) -c memdata.c

error.o: error.c $(HDR)
	$(CC) $(COPT) $(INC) -c error.c

frontier: libfrontier.a main.o
	$(CC) $(COPT) $(LIBDIR) -L. -o frontier main.o $(LIBS) -lfrontier

main.o: main.c $(HDR)
	$(CC) $(COPT) $(INC) -c main.c

frontier-cpp.o: frontier-cpp.cc frontier-cpp.h $(HDR)
	$(CXX) $(COPT) $(CXXOPT) $(INC) -c frontier-cpp.cc

maincc.o: maincc.cc frontier-cpp.h
	$(CXX) $(COPT) $(CXXOPT) $(INC) -c maincc.cc

maincc: libfrontier.a maincc.o frontier-cpp.h
	$(CXX) $(COPT) $(CXXOPT) $(LIBDIR) -L. -o maincc maincc.o $(LIBS) -lfrontier

clean:
	rm -f *.o *.core libfrontier.a frontier maincc
	(cd expat; rm -rf .libs libexpat.la; cd lib && rm -f *.o *.lo && rm -rf .libs _libs;)


