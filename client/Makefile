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
FN_VER_MINOR	=	4.4
COMPILER_TAG    =       gcc_3.2.3
PACKAGE_TAG     =       $(FN_VER_MAJOR).$(FN_VER_MINOR)_cms__$(COMPILER_TAG)
SRC_PACKAGE_TAG =       $(FN_VER_MAJOR).$(FN_VER_MINOR)_cms__src

ifndef EXPAT_DIR
  EXPAT_DIR     =       ./expat/local
  EXPAT_LD_FLAGS=      
  EXPAT_LIBS    =      
else
  EXPAT_LD_FLAGS=      -L${EXPAT_DIR}/lib
  EXPAT_LIBS    =      -lexpat
endif

# GCC settings
FNAPI_VERSION	=	GCC $(FN_VER_MAJOR).$(FN_VER_MINOR)
CC		=	gcc
CXX		=	c++
CXXOPT_LIB	=	-Wall -g -O2 -DFRONTIER_DEBUG -DFNTR_USE_NAMESPACE -DFNTR_USE_EXCEPTIONS -fPIC -DPIC
CXXOPT_APP	=	-Wall -g -O2 -DFRONTIER_DEBUG -DFNTR_USE_NAMESPACE -DFNTR_USE_EXCEPTIONS -fPIC -DPIC
COPT		=	-Wall -g -O2 -DFRONTIER_DEBUG -fPIC -DPIC
LINK_SO		=	c++ -shared -o libfrontier_client.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) -Wl,-soname,libfrontier_client.so.$(FN_VER_MAJOR) $(LIBS)
LINK		=	ar cr libfrontier_client.a

INC		=	-I${EXPAT_DIR}/include -I./include -I.
LIBDIR		=
LIBS		= -lz

OBJ		=	fn-base64.o fn-md5.o fn-zlib.o frontier.o response.o payload.o rs-bin.o memdata.o \
                        frontier_log.o frontier_error.o frontier_config.o fn-mem.o anydata.o frontier-cpp.o FrontierExceptionMapper.o
HDR		=	include/frontier_client/frontier.h fn-internal.h http/fn-htclient.h include/frontier_client/frontier_error.h include/frontier_client/frontier_log.h
HDRXX		=	include/frontier_client/frontier-cpp.h include/frontier_client/FrontierException.hpp include/frontier_client/FrontierExceptionMapper.hpp
BINAPP		=	fn-any fn-req



all: htclient libexpat libfrontier_client.so $(BINAPP)



#####################################################
### BEGIN KIT RELATED STUFF. Note: requires gmake :-(

package := frontier_client
tmpdir := $(shell pwd)/tmp
distdir := $(shell pwd)/dist
files := ./RELEASE_NOTES ./README ./bin/* ./lib/*.so* ./include/frontier_client/*.* ./ups/*.table
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
	(cd `dirname $(src_distfile_gz)`;b=`basename $(src_distfile_gz)`; \
	md5sum $$b >$$b-md5.txt)
	
	
dist-clean: 
	rm -rf $(tmpdir) $(distdir)

$(tmpdir) $(distdir):
	mkdir -p $@

dist: $(distfile_gz)

$(distfile_gz): $(tmpdir) $(distdir) all
	mkdir -p $(distdir)/lib
	mkdir -p $(distdir)/bin
	rm -f $(distdir)/lib/lib*.so*
	cp libfrontier_client.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) $(distdir)/lib
	(cd $(distdir)/lib/ && ln -s libfrontier_client.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) libfrontier_client.so.$(FN_VER_MAJOR) )
	(cd $(distdir)/lib/ && ln -s libfrontier_client.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) libfrontier_client.so )
	cp -r include $(distdir)
	cp -r ups $(distdir)
	cp $(BINAPP) $(distdir)/bin
	cp RELEASE_NOTES $(distdir)
	cp README $(distdir)
	@echo "Making $@"
	rm -f $(distfile); (cd dist; tar cvf $(distfile) $(files))
	rm -f $(distfile_gz); gzip $(distfile)
	(cd `dirname $(distfile_gz)`;b=`basename $(distfile_gz)`; \
	md5sum $$b >$$b-md5.txt)


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
	#$(LINK) .libs/*.o
	ln -s libfrontier_client.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) libfrontier_client.so.$(FN_VER_MAJOR)
	ln -s libfrontier_client.so.$(FN_VER_MAJOR).$(FN_VER_MINOR) libfrontier_client.so


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
	  -lfrontier_client ${EXPAT_LIBS} $(LIBS)

test-b64url.o: test-b64url.c $(HDR)
	$(CC) $(COPT) $(INC) -c test-b64url.c

test-b64url: test-b64url.o
	$(CXX) -g -L. ${EXPAT_LD_FLAGS} -o test-b64url test-b64url.o \
	  -lfrontier_client ${EXPAT_LIBS} $(LIBS)
    
anydata.o: anydata.cc $(HDRXX) $(HDR)
	$(CXX) $(CXXOPT_LIB) $(INC) -c anydata.cc

frontier-cpp.o: frontier-cpp.cc $(HDRXX) $(HDR)
	$(CXX) $(CXXOPT_LIB) $(INC) -c frontier-cpp.cc

maincc.o: maincc.cc $(HDRXX)
	$(CXX) $(CXXOPT_APP) -I./include -c maincc.cc

fn-maincc: maincc.o $(HDRXX)
	$(CXX) $(CXXOPT_APP) -L. ${EXPAT_LD_FLAGS} -o fn-maincc maincc.o \
	  -lfrontier_client ${EXPAT_LIBS} $(LIBS)

maintest.o: maintest.cc $(HDRXX)
	$(CXX) $(CXXOPT_APP) -I./include -c maintest.cc

fn-maintest: maintest.o $(HDRXX)
	$(CXX) $(CXXOPT_APP) -L. ${EXPAT_LD_FLAGS} -o fn-maintest maintest.o \
	  -lfrontier_client ${EXPAT_LIBS} $(LIBS)

FrontierExceptionMapper.o: FrontierExceptionMapper.cpp $(HDRXX)
	$(CXX) $(CXXOPT_APP) -I./include -c FrontierExceptionMapper.cpp

getcids.o: getcids.cc $(HDRXX)
	$(CXX) $(CXXOPT_APP) -Idist/include -c getcids.cc

getcids: all getcids.o $(HDRXX)
	$(CXX) $(CXXOPT_APP) -Ldist/lib ${EXPAT_LD_FLAGS} -o getcids getcids.o \
	  -lfrontier_client ${EXPAT_LIBS} $(LIBS)

request_each.o: request_each.cc $(HDRXX)
	$(CXX) $(CXXOPT_APP) -Idist/include -c request_each.cc

request_each: all request_each.o $(HDRXX)
	$(CXX) $(CXXOPT_APP) -Ldist/lib ${EXPAT_LD_FLAGS} -o request_each request_each.o \
	  -lfrontier_client ${EXPAT_LIBS} $(LIBS)

test-pescalib.o: test-pescalib.cc $(HDRXX)
	$(CXX) $(CXXOPT_APP) -I./include -c test-pescalib.cc

fn-pescalib: test-pescalib.o $(HDRXX)
	$(CXX) $(CXXOPT_APP) -L. ${EXPAT_LD_FLAGS} -o fn-pescalib test-pescalib.o \
	  -lfrontier_client ${EXPAT_LIBS} $(LIBS)

test-any.o: test-any.cc $(HDRXX)
	$(CXX) $(CXXOPT_APP) -I./include -c test-any.cc

fn-any: test-any.o $(HDRXX)
	$(CXX) $(CXXOPT_APP) -L. ${EXPAT_LD_FLAGS} -o fn-any test-any.o \
	  -lfrontier_client ${EXPAT_LIBS} $(LIBS)

test-req.o: test-req.cc $(HDRXX)
	$(CXX) $(CXXOPT_APP) -I./include -c test-req.cc

fn-req: test-req.o $(HDRXX)
	$(CXX) $(CXXOPT_APP) -L. ${EXPAT_LD_FLAGS} -o fn-req test-req.o \
	  -lfrontier_client ${EXPAT_LIBS} $(LIBS)

clean:
	rm -f *.o *.core core libfrontier_client.* frontier main getcids request_each $(BINAPP)
	(cd expat && make clean)
	(cd http && make clean)
	rm -rf .libs
	rm -rf $(tmpdir)
	rm -rf $(distdir)


