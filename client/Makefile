#
# FroNTier client API type 1
# 
# Author: Sergey Kosyakov
#
# $Header$
# 
# $Id$
#

ifeq ($(CXX),KCC)
# KCC settings
CXXOPT		=	--no_exceptions -DFNTR_USE_NAMESPACE
COPT		=	-DFRONTIER_DEBUG
else
# GCC settings
CXX		=	c++
CXXOPT		=	-DFNTR_USE_NAMESPACE -DFNTR_USE_EXCEPTIONS
COPT		=	-pthread -Wall -g -O2 -DFRONTIER_DEBUG
endif

CC		=	cc
INC		=	-Iexpat/lib -I./curl/local/include -I.
LIBDIR		=
LIBS		=

HDR	=	frontier.h fn-internal.h

all: libcurl libfrontier.a distrib

libcurl:
	(cd curl; make all)

libfrontier.a: expat/lib/xmltok.o fn-base64.o fn-md5.o frontier.o response.o payload.o rs-bin.o memdata.o error.o anydata.o frontier-cpp.o
	rm -f libfrontier.a
	ar qcv libfrontier.a frontier.o response.o payload.o rs-bin.o memdata.o error.o expat/lib/xmlparse.o expat/lib/xmlrole.o expat/lib/xmltok.o fn-base64.o fn-md5.o anydata.o frontier-cpp.o
	ranlib libfrontier.a

distrib:
	rm -fr dist/lib dist/include
	mkdir dist/lib
	cp libfrontier.a dist/lib/
	cp curl/local/lib/libcurl.a dist/lib/libfn-curl.a
	mkdir dist/include
	cp frontier-cpp.h dist/include
	cp frontier.h dist/include

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

anydata.o: anydata.cc frontier-cpp.h $(HDR)
	$(CXX) $(COPT) $(CXXOPT) $(INC) -c anydata.cc

frontier-cpp.o: frontier-cpp.cc frontier-cpp.h $(HDR)
	$(CXX) $(COPT) $(CXXOPT) $(INC) -c frontier-cpp.cc

maincc.o: maincc.cc frontier-cpp.h
	$(CXX) -g -Idist/include -c maincc.cc

maincc: all maincc.o frontier-cpp.h
	$(CXX) -g -Ldist/lib -o maincc maincc.o $(LIBS) -lfrontier -lfn-curl

maintest.o: maintest.cc frontier-cpp.h
	$(CXX) -g -Idist/include -c maintest.cc

maintest: all maintest.o frontier-cpp.h
	$(CXX) -g -Ldist/lib -o maintest maintest.o $(LIBS) -lfrontier -lfn-curl

getcids.o: getcids.cc frontier-cpp.h
	$(CXX) -g -Idist/include -c getcids.cc

getcids: all getcids.o frontier-cpp.h
	$(CXX) -g -Ldist/lib -o getcids getcids.o $(LIBS) -lfrontier -lfn-curl

request_each.o: request_each.cc frontier-cpp.h
	$(CXX) -g -Idist/include -c request_each.cc

request_each: all request_each.o frontier-cpp.h
	$(CXX) -g -Ldist/lib -o request_each request_each.o $(LIBS) -lfrontier -lfn-curl

clean:
	rm -f *.o *.core libfrontier.a frontier maincc maintest getcids request_each
	(cd expat; rm -rf .libs libexpat.la; cd lib && rm -f *.o *.lo && rm -rf .libs _libs;)
	(cd expat; rm -f libtool Makefile config.log config.status)
	(cd curl; make clean)
	rm -fr dist/lib dist/include


