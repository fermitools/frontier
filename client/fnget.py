#!/usr/bin/env python

# Simple python frontier client.
# encodes input sql query (standard encoding has to be slightly modified 
# for url safety), retrieves data, and decodes results
# 
#
# example of usage
# ./fnget.py http://lxfs6043.cern.ch:8000/Frontier3D/Frontier "select name,version from frontier_descriptors"
#
#
import sys
import urllib
from xml.dom.minidom import parseString
import base64
import zlib 
import string
import curses.ascii
 

frontierUrl = sys.argv[1]
query = sys.argv[2]

print "Frontier url: ", frontierUrl
print "Query: ", query

# encode query
encQuery = base64.binascii.b2a_base64(zlib.compress(query,9)).replace("+", ".")

# for frontoer request
frontierRequest="%s?type=frontier_request:1:DEFAULT&encoding=BLOB&p1=%s" % (frontierUrl, encQuery)

result = urllib.urlopen(frontierRequest).read()

print "Query result:\n", result
dom = parseString(result)
dataList = dom.getElementsByTagName("data")
# Control characters represent records, but I won't bother with that now,
# and will simply replace those by space.
for data in dataList:
  if data.firstChild is not None:
    row = base64.decodestring(data.firstChild.data)
    newrow = row
    for c in row:
      if c != '\n' and not curses.ascii.isprint(c):
        newrow = newrow.replace(c, ' ')
    print '%s' % (newrow)
 

