#
# FroNTier client API type 1
# 
# Author: Sergey Kosyakov
#
# $Header$
# 
# $Id$
#

FN_VER_MAJOR	=	2
FN_VER_MINOR	=	0.2

ifeq ($(CXX),KCC)
# KCC settings
CC		=	gcc
CXXOPT_LIB	=	-g -O -DFRONTIER_DEBUG --no_exceptions -DFNTR_USE_NAMESPACE -DKCC_COMPILE -fPIC -DPIC
CXXOPT_APP	=	-g -O -DFRONTIER_DEBUG --no_exceptions -DFNTR_USE_NAMESPACE -DKCC_COMPILE --one_per
COPT		=	-Wall -g -O2 -DFRONTIER_DEBUG -fPIC -DPIC
LINK_SO		=	KCC -shared --no_exceptions -o libfrontier_client.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) -Wl,-soname -Wl,libfrontier_client.so.$(FN_VER_MAJOR)
else
# GCC settings
CC		=	gcc
CXX		=	c++
CXXOPT_LIB	=	-Wall -g -O2 -DFRONTIER_DEBUG -DFNTR_USE_NAMESPACE -DFNTR_USE_EXCEPTIONS -fPIC -DPIC
CXXOPT_APP	=	-Wall -g -O2 -DFRONTIER_DEBUG -DFNTR_USE_NAMESPACE -DFNTR_USE_EXCEPTIONS
COPT		=	-Wall -g -O2 -DFRONTIER_DEBUG -fPIC -DPIC
LINK_SO		=	c++ -shared -o libfrontier_client.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) -Wl,-soname,libfrontier_client.so.$(FN_VER_MAJOR)
endif

# Common (hopefully) settings
INC		=	-I./expat/local/include -I./include -I.
LIBDIR		=
LIBS		=

OBJ		=	fn-base64.o fn-md5.o frontier.o response.o payload.o rs-bin.o memdata.o \
                        error.o fn-config.o fn-mem.o anydata.o frontier-cpp.o
HDR		=	include/frontier_client/frontier.h fn-internal.h http/fn-htclient.h
HDRXX		=	include/frontier_client/frontier-cpp.h
BINAPP		=	fn-maincc fn-maintest fn-pescalib fn-any



all: htclient libexpat libfrontier_client.so $(BINAPP)



#####################################################
### BEGIN KIT RELATED STUFF. Note: requires gmake :-(

package := frontier_client
tmpdir := $(shell pwd)/tmp
files := ./RELEASE_NOTES ./README ./bin/* ./lib/*.so* ./include/frontier_client/*.h ./ups/*.table
distfile := $(tmpdir)/$(package).tar
distfile_gz := $(distfile).gz


dist-prepare: $(tmpdir)

$(tmpdir):
	mkdir -p $(tmpdir)

dist: all dist-prepare $(distfile_gz)

$(distfile_gz): all dist-prepare
	rm -fr dist
	mkdir dist
	mkdir dist/lib
	cp libfrontier_client.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) dist/lib/
	(cd dist/lib/ && ln -s libfrontier_client.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) libfrontier_client.so.$(FN_VER_MAJOR) )
	(cd dist/lib/ && ln -s libfrontier_client.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) libfrontier_client.so )
	mkdir dist/include
	mkdir dist/include/frontier_client
	cp -r include dist/
	mkdir dist/bin
	cp $(BINAPP) dist/bin/
	cp RELEASE_NOTES dist/
	cp README dist/
	cp -r ups dist/
	@echo "Making $@"
	rm -f $(distfile); (cd dist; tar cvf $(distfile) $(files))
	rm -f $(distfile_gz); gzip $(distfile)


### END KIT RELATED STUFF. 
#####################################################

htclient:
	(cd http && CC=$(CC) COPT="$(COPT)" make all)

libexpat:
	(cd expat && CC=$(CC) make all)	

libfrontier_client.so: $(OBJ) http/.libs
	rm -f libfrontier_client.so
	rm -f libfrontier_client.so.$(FN_VER_MAJOR)
	rm -f libfrontier_client.so.$(FN_VER_MAJOR).$(FN_VER_MINOR)
	rm -rf .libs
	mkdir .libs
	cp ./http/.libs/*.o .libs
	cp ./expat/expat-1.95.7/lib/.libs/*.o .libs
	cp $(OBJ) .libs
	$(LINK_SO) .libs/*.o
	ln -s libfrontier_client.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) libfrontier_client.so.$(FN_VER_MAJOR)
	ln -s libfrontier_client.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) libfrontier_client.so


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

main.o: main.c $(HDR)
	$(CC) $(COPT) $(INC) -c main.c

main: all main.o
	$(CXX) -g -Ldist/lib -o main main.o $(LIBS) -lfrontier

anydata.o: anydata.cc $(HDRXX) $(HDR)
	$(CXX) $(CXXOPT_LIB) $(INC) -c anydata.cc

frontier-cpp.o: frontier-cpp.cc $(HDRXX) $(HDR)
	$(CXX) $(CXXOPT_LIB) $(INC) -c frontier-cpp.cc

maincc.o: maincc.cc $(HDRXX)
	$(CXX) $(CXXOPT_APP) -I./include -c maincc.cc

fn-maincc: maincc.o $(HDRXX)
	$(CXX) $(CXXOPT_APP) -L. -o fn-maincc maincc.o $(LIBS) -lfrontier_client

maintest.o: maintest.cc $(HDRXX)
	$(CXX) $(CXXOPT_APP) -I./include -c maintest.cc

fn-maintest: maintest.o $(HDRXX)
	$(CXX) $(CXXOPT_APP) -L. -o fn-maintest maintest.o $(LIBS) -lfrontier_client

getcids.o: getcids.cc $(HDRXX)
	$(CXX) $(CXXOPT_APP) -Idist/include -c getcids.cc

getcids: all getcids.o $(HDRXX)
	$(CXX) $(CXXOPT_APP) -Ldist/lib -o getcids getcids.o $(LIBS) -lfrontier_client

request_each.o: request_each.cc $(HDRXX)
	$(CXX) $(CXXOPT_APP) -Idist/include -c request_each.cc

request_each: all request_each.o $(HDRXX)
	$(CXX) $(CXXOPT_APP) -Ldist/lib -o request_each request_each.o $(LIBS) -lfrontier_client

test-pescalib.o: test-pescalib.cc $(HDRXX)
	$(CXX) $(CXXOPT_APP) -I./include -c test-pescalib.cc

fn-pescalib: test-pescalib.o $(HDRXX)
	$(CXX) $(CXXOPT_APP) -L. -o fn-pescalib test-pescalib.o $(LIBS) -lfrontier_client

test-any.o: test-any.cc $(HDRXX)
	$(CXX) $(CXXOPT_APP) -I./include -c test-any.cc

fn-any: test-any.o $(HDRXX)
	$(CXX) $(CXXOPT_APP) -L. -o fn-any test-any.o $(LIBS) -lfrontier_client

clean:
	rm -f *.o *.core core libfrontier_client.* frontier main getcids request_each $(BINAPP)
	(cd expat && make clean)
	(cd http && make clean)
	rm -fr dist
	rm -rf .libs
	rm -rf ti_files
	rm -rf $(tmpdir)


