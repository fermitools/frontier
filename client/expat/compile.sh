#!/bin/sh

/bin/sh ./libtool  --mode=compile cc -g -O2 -Wall -Wmissing-prototypes -Wstrict-prototypes -DHAVE_EXPAT_CONFIG_H   -I./lib -I. -o lib/xmlparse.lo -c lib/xmlparse.c
/bin/sh ./libtool  --mode=compile cc -g -O2 -Wall -Wmissing-prototypes -Wstrict-prototypes -DHAVE_EXPAT_CONFIG_H   -I./lib -I. -o lib/xmltok.lo -c lib/xmltok.c
/bin/sh ./libtool  --mode=compile cc -g -O2 -Wall -Wmissing-prototypes -Wstrict-prototypes -DHAVE_EXPAT_CONFIG_H   -I./lib -I. -o lib/xmlrole.lo -c lib/xmlrole.c

/bin/sh ./libtool --dry-run  --mode=link cc -g -O2 -Wall -Wmissing-prototypes -Wstrict-prototypes -DHAVE_EXPAT_CONFIG_H   -I./lib -I. -no-undefined -version-info 5:0:5 -rpath /home/serge/work/prog/cvs/frontier/client/expat/../local/lib  -o libexpat.la lib/xmlparse.lo lib/xmltok.lo lib/xmlrole.lo


