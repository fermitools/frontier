#!/usr/bin/env python
'''Simple Frontier Client built on top of the Frontier Python DBAPI 2.0

Usage example:

    $ ./frontier_simple_client.py "select 'The ' || :message || ' is' as message, 1 + :two as result from dual where :three < 4" result 2 3
    Total rows = 1
    Columns description:
       MESSAGE of type VARCHAR2
       RESULT of type NUMBER
    Rows:
       ['The result is', 3]
'''
from __future__ import print_function

__author__ = 'Miguel Ojeda'
__copyright__ = 'Copyright 2013, CERN'
__credits__ = ['Miguel Ojeda']
__license__ = 'Unknown'
__maintainer__ = 'Miguel Ojeda'
__email__ = 'mojedasa@cern.ch'


import optparse
import logging

import frontier


if __name__ == '__main__':
    parser = optparse.OptionParser()
    parser.add_option('-v', '--verbose', action='store_true', dest='verbose')
    parser.add_option('-d', '--debug', action='store_true', dest='debug')
    (options, args) = parser.parse_args()

    logging.basicConfig(
        format = '[%(asctime)s] %(levelname)s: %(message)s',
        level = logging.DEBUG,
    )

    if options.verbose:
        frontier.logger.setLevel(logging.INFO)
    if options.debug:
        frontier.logger.setLevel(logging.DEBUG)
        frontier.frontier_client.logger.setLevel(logging.DEBUG)

    conn = frontier.connect()
    c = conn.cursor()
    c.execute(args[0], args[1:])

    print('Total rows = %s' % c.rowcount)

    print('Columns description:')
    for (name, data_type, _, _, _, _, _) in c.description:
        print('   %s of type %s' % (name, data_type))

    print('Rows:')
    for row in c.fetchall():
        print('   %s' % row)

    c.close()
    conn.close()

