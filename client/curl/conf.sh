#!/bin/sh

# Config for curl which will be used in the frontier
# Author: Sergey Kosyakov
# $Id$
#

INST_DIR=`pwd`/local

(cd curl-7.11.1; ./configure --prefix=$INST_DIR --disable-dependency-tracking --disable-largefile --disable-shared --disable-ftp --disable-gopher --disable-file --disable-ldap --disable-dict --disable-telnet --disable-manual --disable-libgcc --disable-ipv6 --enable-thread --disable-ares --without-pic --without-ssl --without-ca-bundle --without-zlib)


