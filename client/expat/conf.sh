#!/bin/sh

# Config for curl which will be used in the frontier
# Author: Sergey Kosyakov
# $Id$
#

INST_DIR=`pwd`/local

(cd expat-1.95.7; /bin/sh configure --prefix=`pwd`/../local --disable-static --enable-shared --with-pic)

