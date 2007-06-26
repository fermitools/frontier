#
# Author: Sergey Kosyakov
#
# $Id$
#

# The purpose here is to build expat only if it is not already provided.
ifdef EXPAT_DIR
all clean:
else
  EXPAT_DIR=local
  EXPAT_INCLUDE_DIR=${EXPAT_DIR}/include
  EXPAT_LIB_OBJECTS_DIR=${EXPAT_DIR}/lib_objects
  EXPAT_SRC_DIR=expat-2.0.1
  EXPAT_SRC_TAR=${EXPAT_SRC_DIR}.tar.gz
  # need the LOBJS to build using libtool, needed at least for 64-bit and PIC
  EXPAT_LOBJS=lib/xmlparse.lo lib/xmlrole.lo lib/xmltok.lo
  EXPAT_OBJS=lib/.libs/xmlparse.o lib/.libs/xmlrole.o lib/.libs/xmltok.o

all: ${EXPAT_DIR}/built

${EXPAT_DIR}/built:
	# The following builds a local copy of a portion of the expat library
	@echo "Rebuilding local expat"
	rm -rf ${EXPAT_DIR} 
	mkdir -p ${EXPAT_DIR}
	mkdir -p ${EXPAT_LIB_OBJECTS_DIR}
	gunzip -c ${EXPAT_SRC_TAR} | tar xf -
	cd ${EXPAT_SRC_DIR} && /bin/sh configure --prefix=`pwd`/local --disable-static --enable-shared --with-pic
	cd ${EXPAT_SRC_DIR} && make $(EXPAT_LOBJS) && cp $(EXPAT_OBJS) ../${EXPAT_LIB_OBJECTS_DIR}
	mkdir -p ${EXPAT_INCLUDE_DIR}
	cp ${EXPAT_SRC_DIR}/lib/*.h ${EXPAT_INCLUDE_DIR}
	touch ${EXPAT_DIR}/built
	
clean:
	rm -rf ${EXPAT_SRC_DIR} 
	rm -rf local

endif
