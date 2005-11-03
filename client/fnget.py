#!/usr/bin/env python

import sys
import urllib
from xml.dom.minidom import parseString
import base64
import zlib 
 

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
for data in dataList:
  if data.firstChild is not None:
    print base64.decodestring(data.firstChild.data)


