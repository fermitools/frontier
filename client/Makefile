#
# FroNTier client API type 1
# 
# Author: Sergey Kosyakov
#
# $Header$
# 
# $Id$
#

FN_VER_MAJOR	=	1
FN_VER_MINOR	=	0.0

ifeq ($(CXX),KCC)
# KCC settings
CXXOPT_LIB	=	-g -O -DFRONTIER_DEBUG --no_exceptions -DFNTR_USE_NAMESPACE -DKCC_COMPILE -fPIC -DPIC
CXXOPT_APP	=	-g -O -DFRONTIER_DEBUG --no_exceptions -DFNTR_USE_NAMESPACE -DKCC_COMPILE --one_per
COPT		=	-Wall -g -O2 -DFRONTIER_DEBUG -fPIC -DPIC
LINK_SO		=	KCC -shared --no_exceptions -o libfrontier.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) -Wl,-soname -Wl,libfrontier.so.$(FN_VER_MAJOR)
else
# GCC settings
CXX		=	c++
CXXOPT_LIB	=	-Wall -g -O2 -DFRONTIER_DEBUG -DFNTR_USE_NAMESPACE -DFNTR_USE_EXCEPTIONS -fPIC -DPIC
CXXOPT_APP	=	-Wall -g -O2 -DFRONTIER_DEBUG -DFNTR_USE_NAMESPACE -DFNTR_USE_EXCEPTIONS
COPT		=	-Wall -g -O2 -DFRONTIER_DEBUG -fPIC -DPIC
LINK_SO		=	cc -shared -o libfrontier.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) -Wl,-soname,libfrontier.so.$(FN_VER_MAJOR)
endif

# Common (hopefully) settings
CC		=	cc
INC		=	-I./expat/local/include -I.
LIBDIR		=
LIBS		=

OBJ		=	fn-base64.o fn-md5.o frontier.o response.o payload.o rs-bin.o memdata.o \
                        error.o fn-config.o fn-mem.o anydata.o frontier-cpp.o
HDR		=	frontier.h fn-internal.h http/fn-htclient.h

all: htclient libexpat libfrontier.so distrib

htclient:
	(cd http && CC=$(CC) COPT="$(COPT)" make all)

libexpat:
	(cd expat && make all)	

libfrontier.so: $(OBJ) http/.libs
	rm -f libfrontier.so
	rm -f libfrontier.so.$(FN_VER_MAJOR)
	rm -f libfrontier.so.$(FN_VER_MAJOR).$(FN_VER_MINOR)
	rm -rf .libs
	mkdir .libs
	cp ./http/.libs/*.o .libs
	cp ./expat/expat-1.95.7/lib/.libs/*.o .libs
	cp $(OBJ) .libs
	$(LINK_SO) .libs/*.o
	ln -s libfrontier.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) libfrontier.so.$(FN_VER_MAJOR)
	ln -s libfrontier.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) libfrontier.so

distrib:
	rm -fr dist
	mkdir dist
	mkdir dist/lib
	cp libfrontier.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) dist/lib/
	(cd dist/lib/ && ln -s libfrontier.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) libfrontier.so.$(FN_VER_MAJOR) )
	(cd dist/lib/ && ln -s libfrontier.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) libfrontier.so )
	mkdir dist/include
	cp frontier-cpp.h dist/include
	cp frontier.h dist/include


fn-base64.o: fn-base64.c $(HDR)
	$(CC) $(COPT) $(INC) -c fn-base64.c

fn-md5.o: fn-md5.c $(HDR)
	$(CC) $(COPT) $(INC) -c fn-md5.c

fn-mem.o: fn-mem.c $(HDR)
	$(CC) $(COPT) $(INC) -c fn-mem.c

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

fn-config.o: fn-config.c $(HDR)
	$(CC) $(COPT) $(INC) -c fn-config.c

frontier: libfrontier.a main.o
	$(CC) $(COPT) $(LIBDIR) -L. -o frontier main.o $(LIBS) -lfrontier

main.o: main.c $(HDR)
	$(CC) $(COPT) $(INC) -c main.c

main: all main.o
	$(CXX) -g -Ldist/lib -o main main.o $(LIBS) -lfrontier

anydata.o: anydata.cc frontier-cpp.h $(HDR)
	$(CXX) $(CXXOPT_LIB) $(INC) -c anydata.cc

frontier-cpp.o: frontier-cpp.cc frontier-cpp.h $(HDR)
	$(CXX) $(CXXOPT_LIB) $(INC) -c frontier-cpp.cc

maincc.o: maincc.cc frontier-cpp.h
	$(CXX) $(CXXOPT_APP) -Idist/include -c maincc.cc

maincc: all maincc.o frontier-cpp.h
	$(CXX) $(CXXOPT_APP) -Ldist/lib -o maincc maincc.o $(LIBS) -lfrontier

maintest.o: maintest.cc frontier-cpp.h
	$(CXX) $(CXXOPT_APP) -Idist/include -c maintest.cc

maintest: all maintest.o frontier-cpp.h
	$(CXX) $(CXXOPT_APP) -Ldist/lib -o maintest maintest.o $(LIBS) -lfrontier

getcids.o: getcids.cc frontier-cpp.h
	$(CXX) $(CXXOPT_APP) -Idist/include -c getcids.cc

getcids: all getcids.o frontier-cpp.h
	$(CXX) $(CXXOPT_APP) -Ldist/lib -o getcids getcids.o $(LIBS) -lfrontier

request_each.o: request_each.cc frontier-cpp.h
	$(CXX) $(CXXOPT_APP) -Idist/include -c request_each.cc

request_each: all request_each.o frontier-cpp.h
	$(CXX) $(CXXOPT_APP) -Ldist/lib -o request_each request_each.o $(LIBS) -lfrontier

test-pescalib.o: test-pescalib.cc frontier-cpp.h
	$(CXX) $(CXXOPT_APP) -Idist/include -c test-pescalib.cc

test-pescalib: all test-pescalib.o frontier-cpp.h
	$(CXX) $(CXXOPT_APP) -Ldist/lib -o test-pescalib test-pescalib.o $(LIBS) -lfrontier

test-any.o: test-any.cc frontier-cpp.h
	$(CXX) $(CXXOPT_APP) -Idist/include -c test-any.cc

test-any: all test-any.o frontier-cpp.h
	$(CXX) $(CXXOPT_APP) -Ldist/lib -o test-any test-any.o $(LIBS) -lfrontier

clean:
	rm -f *.o *.core libfrontier.* frontier main maincc maintest getcids request_each \
              test-pescalib test-any
	(cd expat && make clean)
	(cd http && make clean)
	rm -fr dist
	rm -rf .libs
	rm -rf ti_files


