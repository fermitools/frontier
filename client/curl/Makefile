#
# Author: Sergey Kosyakov
#
# $Id$
#

all: local/lib/libcurl.a


local/lib/libcurl.a:
	gunzip -c curl-7.11.1.tar.gz|tar xvf -
	sh conf.sh
	(cd curl-7.11.1;make && make install)

clean:
	rm -rf curl-7.11.1
	rm -rf local/bin
	rm -rf local/include
	rm -rf local/man
	rm -rf local/lib/libcurl.la
	rm -rf local/lib/libcurl.a

