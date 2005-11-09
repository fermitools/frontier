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

#print "Query result:\n", result
dom = parseString(result)
dataList = dom.getElementsByTagName("data")
# Control characters represent records, but I won't bother with that now,
# and will simply replace those by space.
for data in dataList:
  if data.firstChild is not None:

    row = base64.decodestring(data.firstChild.data)
    for c in [ '\x00', '\x01', '\x02', '\x03', '\x04', '\x06', '\x08', '\x09', '\x0a', '\x0b', '\x0c', '\x0d'  ]:
      row = row.replace(c, ' ')

    print "\nFields: "
    endFirstRow = row.find('\x07')
    firstRow = row[:endFirstRow]
    for c in firstRow:
      if curses.ascii.isctrl(c):
         firstRow = firstRow.replace(c, '\n')
    print firstRow

    print "\nRecords:"
    pos = endFirstRow + 1
    while True:
      newrow = row[pos:]
      endRow = newrow.find('\x07')
      if endRow < 0:
        break
      fixedRow = newrow[:endRow]
      pos = pos + endRow + 1
      fixedRow = fixedRow.replace('\n', '')
      print fixedRow
    


