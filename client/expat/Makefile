#
# Author: Sergey Kosyakov
#
# $Id$
#

all: local/lib/libfn-expat.so.1.0.0


local/lib/libfn-expat.so.1.0.0:
	rm -rf local
	mkdir local
	mkdir local/lib
	gunzip -c expat-1.95.7.tar.gz|tar xf -
	(cd expat-1.95.7/lib; patch < ../../patch_expat_h)
	sh conf.sh
	(cd expat-1.95.7/lib; mkdir .libs; for i in `cat ../../lib.list`; do bn=`basename $$i .c`; echo $$i; cc -Wall -Wmissing-prototypes -Wstrict-prototypes -DHAVE_EXPAT_CONFIG_H -I. -I../ -g -O2 -c $$i -fPIC -DPIC -o .libs/$$bn.o;  done; cc -shared -o libfn-expat.so.1.0.0 -Wl,-soname,libfn-expat.so.1 .libs/*.o ; cp libfn-expat.so.1.0.0 ../../local/lib)
	mkdir local/include
	cp expat-1.95.7/lib/*.h local/include/
	
	
#	cp .libs/libfn-curl.so.2.0.2 ../../local/lib)

clean:
	rm -rf expat-1.95.7
	rm -rf local

