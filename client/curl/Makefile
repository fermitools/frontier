#
# Author: Sergey Kosyakov
#
# $Id$
#

all: local/lib/libfn-curl.so.1.0.0


local/lib/libfn-curl.so.1.0.0:
	rm -rf local
	mkdir local
	mkdir local/lib
	gunzip -c curl-7.11.1.tar.gz|tar xf -
	sh conf.sh
	(cd curl-7.11.1/lib; mkdir .libs; for i in `cat ../../lib.list`; do bn=`basename $$i .c`; echo $$i; cc -DHAVE_CONFIG_H -I../include -I../lib -I../lib -g -O2 -c $$i -fPIC -DPIC -o .libs/$$bn.o;  done; cc -shared -o libfn-curl.so.1.0.0 -Wl,-soname,libfn-curl.so.1 .libs/*.o ; cp libfn-curl.so.1.0.0 ../../local/lib)
	(cd curl-7.11.1/include; make install)
	
	
#	cp .libs/libfn-curl.so.2.0.2 ../../local/lib)

clean:
	rm -rf curl-7.11.1
	rm -rf local

