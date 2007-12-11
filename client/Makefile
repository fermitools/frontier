#
# frontier client Makefile
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

FN_VER_MAJOR	=	2
FN_VER_MINOR	=	7.5
COMPILER_TAG    =       gcc_3.2.3
PACKAGE_TAG     =       $(FN_VER_MAJOR).$(FN_VER_MINOR)__$(COMPILER_TAG)
SRC_PACKAGE_TAG =       $(FN_VER_MAJOR).$(FN_VER_MINOR)__src

ifndef EXPAT_DIR
  EXPAT_DIR     =       ./expat/local
  EXPAT_LD_FLAGS=      
  EXPAT_LIBS    =      
else
  EXPAT_LD_FLAGS=      -L${EXPAT_DIR}/lib
  EXPAT_LIBS    =      -lexpat
endif

ifndef ZLIB_DIR
  ZLIB_LD_FLAGS=
  ZLIB_INC     =
else
  ZLIB_LD_FLAGS=      -L${ZLIB_DIR}/lib
  ZLIB_INC     =      -I${ZLIB_DIR}/include
endif

ifndef OPENSSL_DIR
  OPENSSL_LD_FLAGS=
  OPENSSL_INC     =
else
  OPENSSL_LD_FLAGS=   -L${OPENSSL_DIR}/lib
  OPENSSL_INC     =   -I${OPENSSL_DIR}/include
endif

# Mac OSX uses "dylib" instead of "so" for shared dynamic libraries
DYLIBTYPE := $(shell if [ -f /usr/lib/libc.dylib ]; then echo dylib; else echo so; fi)

# GCC settings
DEBUG_OPTIM     =       -g -O2
FNAPI_VERSION	=	$(FN_VER_MAJOR).$(FN_VER_MINOR)
CC		=	gcc
CXX		=	c++
CXXOPT_LIB	=	-Wall $(DEBUG_OPTIM) -DFRONTIER_DEBUG -DFNTR_USE_NAMESPACE -DFNTR_USE_EXCEPTIONS -fPIC -DPIC
CXXOPT_APP	=	-Wall $(DEBUG_OPTIM) -DFRONTIER_DEBUG -DFNTR_USE_NAMESPACE -DFNTR_USE_EXCEPTIONS -fPIC -DPIC
COPT		=	-Wall $(DEBUG_OPTIM) -DFRONTIER_DEBUG -fPIC -DPIC
LINK_SO         =       c++ -shared -o libfrontier_client.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) -Wl,-soname,libfrontier_client.so.$(FN_VER_MAJOR) $(LIBS)
LINK_DYLIB	=	c++ -dynamiclib -flat_namespace  -compatibility_version $(FN_VER_MAJOR) -current_version $(FN_VER_MAJOR).$(FN_VER_MINOR) -o libfrontier_client.$(FN_VER_MAJOR).$(FN_VER_MINOR).dylib -undefined suppress -single_module
LINK		=	ar cr libfrontier_client.a

INC		=	-I${EXPAT_DIR}/include ${OPENSSL_INC} ${ZLIB_INC} -I./include -I.
LIBDIR		=
LIBS		= 	${OPENSSL_LD_FLAGS} -lssl ${ZLIB_LD_FLAGS} -lz 

OBJ		=	fn-base64.o fn-md5.o fn-zlib.o frontier.o response.o payload.o rs-bin.o memdata.o \
                        frontier_log.o frontier_error.o frontier_config.o fn-mem.o anydata.o frontier-cpp.o FrontierExceptionMapper.o
HDR		=	include/frontier_client/frontier.h fn-internal.h http/fn-htclient.h include/frontier_client/frontier_error.h include/frontier_client/frontier_log.h
HDRXX		=	include/frontier_client/frontier-cpp.h include/frontier_client/FrontierException.hpp include/frontier_client/FrontierExceptionMapper.hpp
BINAPP		=	fn-any fn-req

ifeq ($(DYLIBTYPE),dylib)
   LIBCLIENT	= 	libfrontier_client.dylib
   STATICBINAPP =
else
   LIBCLIENT	=	libfrontier_client.so
   STATICBINAPP	=	fn-req.static
endif



all: htclient libexpat $(LIBCLIENT) $(BINAPP) $(STATICBINAPP)



#####################################################
### BEGIN KIT RELATED STUFF. Note: requires gmake :-(

package := frontier_client
tmpdir := $(shell pwd)/tmp
distdir := $(shell pwd)/dist
files := ./RELEASE_NOTES ./README ./bin/* ./lib/*.$(DYLIBTYPE)* ./include/frontier_client/*.* ./ups/*.table
src_files = `sh -c "find . \\( \\( -name CVS -o -name SCCS -o -name tmp \\) -prune \\) -o \\( -type d -links 2 -print -prune \\) -o \\! -type d ! -name '.manifest*' ! \\( -name core -type f \\)  -print 2>/dev/null | sort"`
distfile := $(tmpdir)/$(package)__$(PACKAGE_TAG).tar
distfile_gz := $(distfile).gz
src_distdir := $(package)__$(SRC_PACKAGE_TAG)
src_distfile := $(tmpdir)/$(src_distdir).tar
src_distfile_gz := $(src_distfile).gz

# Build all distributions from scratch:
all-dist: clean src-dist dist

# Source distribution.
src-dist: clean $(tmpdir)
	(cd expat && make clean)
	(cd http && make clean)
	tar cvf $(src_distfile) $(src_files)
	mkdir -p $(tmpdir)/$(src_distdir)
	(cd $(tmpdir)/$(src_distdir) && tar xvf $(src_distfile))
	rm -f $(src_distfile)
	(cd $(tmpdir) && tar zcvf $(src_distfile_gz) $(src_distdir))
	rm -rf $(tmpdir)/$(src_distdir)
	
	
dist-clean: 
	rm -rf $(tmpdir) $(distdir)

$(tmpdir) $(distdir):
	mkdir -p $@

dist: $(distfile_gz)

$(distfile_gz): $(tmpdir) $(distdir) all
	mkdir -p $(distdir)/lib
	mkdir -p $(distdir)/bin
	rm -f $(distdir)/lib/lib*.$(DYLIBTYPE)*
ifeq ($(DYLIBTYPE),dylib)
	cp libfrontier_client.$(FN_VER_MAJOR).$(FN_VER_MINOR).dylib $(distdir)/lib
	(cd $(distdir)/lib/ && ln -s libfrontier_client.$(FN_VER_MAJOR).$(FN_VER_MINOR).dylib libfrontier_client.$(FN_VER_MAJOR).dylib )
	(cd $(distdir)/lib/ && ln -s libfrontier_client.$(FN_VER_MAJOR).$(FN_VER_MINOR).dylib libfrontier_client.dylib )
else
	cp libfrontier_client.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) $(distdir)/lib
	(cd $(distdir)/lib/ && ln -s libfrontier_client.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) libfrontier_client.so.$(FN_VER_MAJOR) )
	(cd $(distdir)/lib/ && ln -s libfrontier_client.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) libfrontier_client.so )
endif
	cp -r include $(distdir)
	cp -r ups $(distdir)
	cp $(BINAPP) $(distdir)/bin
	cp RELEASE_NOTES $(distdir)
	cp README $(distdir)
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
	if [ -d ./expat/local/lib_objects ]; then cp ./expat/local/lib_objects/*.o .libs; fi
	cp $(OBJ) .libs
	$(LINK_SO) .libs/*.o
	ln -s libfrontier_client.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) libfrontier_client.so.$(FN_VER_MAJOR)
	ln -s libfrontier_client.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) libfrontier_client.so

libfrontier_client.dylib: $(OBJ) http/.libs
	rm -f libfrontier_client.dylib
	rm -f libfrontier_client.$(FN_VER_MAJOR).dylib
	rm -f libfrontier_client.$(FN_VER_MAJOR).$(FN_VER_MINOR).dylib
	rm -rf .libs
	mkdir .libs
	cp ./http/.libs/*.o .libs
	if [ -d ./expat/local/lib_objects ]; then cp ./expat/local/lib_objects/*.o .libs; fi
	cp $(OBJ) .libs
	$(LINK_DYLIB) .libs/*.o
	ln -s libfrontier_client.$(FN_VER_MAJOR).$(FN_VER_MINOR).dylib libfrontier_client.$(FN_VER_MAJOR).dylib
	ln -s libfrontier_client.$(FN_VER_MAJOR).$(FN_VER_MINOR).dylib libfrontier_client.dylib


fn-base64.o: fn-base64.c $(HDR) fn-base64.h
	$(CC) $(COPT) $(INC) -c fn-base64.c

fn-md5.o: fn-md5.c $(HDR)
	$(CC) $(COPT) $(INC) -c fn-md5.c

fn-zlib.o: fn-zlib.c $(HDR) fn-zlib.h fn-base64.h
	$(CC) $(COPT) $(INC) -c fn-zlib.c
    
fn-mem.o: fn-mem.c $(HDR)
	$(CC) $(COPT) $(INC) -c fn-mem.c

frontier.o: frontier.c $(HDR)
	$(CC) -DFNAPI_VERSION="\"$(FNAPI_VERSION)\"" $(COPT) $(INC) -c frontier.c

response.o: response.c $(HDR)
	$(CC) $(COPT) $(INC) -c response.c

payload.o: payload.c $(HDR)
	$(CC) $(COPT) $(INC) -c payload.c

rs-bin.o: rs-bin.c $(HDR)
	$(CC) $(COPT) $(INC) -c rs-bin.c

memdata.o: memdata.c $(HDR)
	$(CC) $(COPT) $(INC) -c memdata.c

frontier_error.o: frontier_error.c include/frontier_client/frontier_error.h include/frontier_client/frontier_log.h
	$(CC) $(COPT) $(INC) -c frontier_error.c

frontier_log.o: frontier_log.c include/frontier_client/frontier_log.h 
	$(CC) $(COPT) $(INC) -c frontier_log.c

frontier_config.o: frontier_config.c $(HDR)
	$(CC) $(COPT) $(INC) -c frontier_config.c

main.o: main.c $(HDR)
	$(CC) $(COPT) $(INC) -c main.c

main: all main.o
	$(CXX) -g -Ldist/lib ${EXPAT_LD_FLAGS} -o main main.o \
	  -L. -lfrontier_client ${EXPAT_LIBS} $(LIBS)

test-b64url.o: test-b64url.c $(HDR)
	$(CC) $(COPT) $(INC) -c test-b64url.c

test-b64url: test-b64url.o
	$(CXX) -g -L. ${EXPAT_LD_FLAGS} -o test-b64url test-b64url.o \
	  -L. -lfrontier_client ${EXPAT_LIBS} $(LIBS)
    
anydata.o: anydata.cc $(HDRXX) $(HDR)
	$(CXX) $(CXXOPT_LIB) $(INC) -c anydata.cc

frontier-cpp.o: frontier-cpp.cc $(HDRXX) $(HDR)
	$(CXX) $(CXXOPT_LIB) $(INC) -c frontier-cpp.cc

maincc.o: maincc.cc $(HDRXX)
	$(CXX) $(CXXOPT_APP) -I./include -c maincc.cc

fn-maincc: maincc.o $(HDRXX)
	$(CXX) $(CXXOPT_APP) -L. ${EXPAT_LD_FLAGS} -o fn-maincc maincc.o \
	  -L. -lfrontier_client ${EXPAT_LIBS} $(LIBS)

maintest.o: maintest.cc $(HDRXX)
	$(CXX) $(CXXOPT_APP) -I./include -c maintest.cc

fn-maintest: maintest.o $(HDRXX)
	$(CXX) $(CXXOPT_APP) -L. ${EXPAT_LD_FLAGS} -o fn-maintest maintest.o \
	  -L. -lfrontier_client ${EXPAT_LIBS} $(LIBS)

FrontierExceptionMapper.o: FrontierExceptionMapper.cpp $(HDRXX)
	$(CXX) $(CXXOPT_APP) -I./include -c FrontierExceptionMapper.cpp

getcids.o: getcids.cc $(HDRXX)
	$(CXX) $(CXXOPT_APP) -Idist/include -c getcids.cc

getcids: all getcids.o $(HDRXX)
	$(CXX) $(CXXOPT_APP) -Ldist/lib ${EXPAT_LD_FLAGS} -o getcids getcids.o \
	  -L. -lfrontier_client ${EXPAT_LIBS} $(LIBS)

request_each.o: request_each.cc $(HDRXX)
	$(CXX) $(CXXOPT_APP) -Idist/include -c request_each.cc

request_each: all request_each.o $(HDRXX)
	$(CXX) $(CXXOPT_APP) -Ldist/lib ${EXPAT_LD_FLAGS} -o request_each request_each.o \
	  -L. -lfrontier_client ${EXPAT_LIBS} $(LIBS)

test-pescalib.o: test-pescalib.cc $(HDRXX)
	$(CXX) $(CXXOPT_APP) -I./include -c test-pescalib.cc

fn-pescalib: test-pescalib.o $(HDRXX)
	$(CXX) $(CXXOPT_APP) -L. ${EXPAT_LD_FLAGS} -o fn-pescalib test-pescalib.o \
	  -L. -lfrontier_client ${EXPAT_LIBS} $(LIBS)

test-any.o: test-any.cc $(HDRXX)
	$(CXX) $(CXXOPT_APP) -I./include -c test-any.cc

fn-any: test-any.o $(HDRXX)
	$(CXX) $(CXXOPT_APP) -L. ${EXPAT_LD_FLAGS} -o fn-any test-any.o \
	  -L. -lfrontier_client ${EXPAT_LIBS} $(LIBS)

test-req.o: test-req.cc $(HDRXX)
	$(CXX) $(CXXOPT_APP) -I./include -c test-req.cc

fn-req: test-req.o $(HDRXX)
	$(CXX) $(CXXOPT_APP) -L. ${EXPAT_LD_FLAGS} -o fn-req test-req.o \
	  -L. -lfrontier_client ${EXPAT_LIBS} $(LIBS)

# staticly linked app works better in gdb
fn-req.static: libfrontier_client.so fn-req
	$(CXX) $(CXXOPT_APP) -L. ${EXPAT_LD_FLAGS} -o fn-req.static test-req.o \
	  .libs/*.o ${EXPAT_LIBS} $(LIBS)


clean:
	rm -f *.o *.core core libfrontier_client.* frontier main getcids request_each $(BINAPP) $(STATICBINAPP)
	(cd expat && make clean)
	(cd http && make clean)
	rm -rf .libs
	rm -rf $(tmpdir)
	rm -rf $(distdir)


