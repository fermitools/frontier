#
# Author: Sergey Kosyakov
#
# $Id$
#

# The purpose here is to build expat only if it is not already provided.
ifndef EXPAT_DIR
  EXPAT_DIR=local
  EXPAT_INCLUDE_DIR=${EXPAT_DIR}/include
  EXPAT_LIB_DIR=${EXPAT_DIR}/lib
  EXPAT_LIB_OBJECTS_DIR=${EXPAT_DIR}/lib_objects
  EXPAT_SRC_DIR=expat-1.95.8
  EXPAT_SRC_TAR=${EXPAT_SRC_DIR}.tar.gz
  EXPAT_SOLIB_NAME=libfn-expat.so
else
  EXPAT_SOLIB_NAME=libexpat.so
endif

all: ${EXPAT_DIR}/lib/${EXPAT_SOLIB_NAME}

${EXPAT_DIR}/lib/${EXPAT_SOLIB_NAME}:
	# The following builds local copy of the expat libraries.
	echo "Using expat directory: ${EXPAT_DIR}"
	echo "Rebuilding $(notdir $@)"
	rm -rf ${EXPAT_DIR} 
	mkdir -p ${EXPAT_LIB_DIR}
	mkdir -p ${EXPAT_LIB_OBJECTS_DIR}
	gunzip -c ${EXPAT_SRC_TAR} | tar xf -
	(cd ${EXPAT_SRC_DIR}; /bin/sh configure --prefix=`pwd`/local --disable-static --enable-shared --with-pic)
	(cd ${EXPAT_SRC_DIR}/lib; for i in `cat ../../lib.list`; do bn=`basename $$i .c`; echo $$i; $(CC) -Wall -Wmissing-prototypes -Wstrict-prototypes -DHAVE_EXPAT_CONFIG_H -I. -I../ -g -O2 -c $$i -fPIC -DPIC -o $$bn.o;  done; cc -shared -o ${EXPAT_SOLIB_NAME}.1.0.0 -Wl,-soname,${EXPAT_SOLIB_NAME}.1 *.o ; cp ${EXPAT_SOLIB_NAME}.1.0.0 ../../${EXPAT_LIB_DIR}; cp *.o ../../${EXPAT_LIB_OBJECTS_DIR} ; cd ../../${EXPAT_LIB_DIR}; ln -s ${EXPAT_SOLIB_NAME}.1.0.0 ${EXPAT_SOLIB_NAME})
	mkdir -p ${EXPAT_INCLUDE_DIR}
	cp ${EXPAT_SRC_DIR}/lib/*.h ${EXPAT_INCLUDE_DIR}
	
clean:
	rm -rf ${EXPAT_SRC_DIR} 
	rm -rf local

