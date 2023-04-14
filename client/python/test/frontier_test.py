#!/usr/bin/env python
'''Frontier Python DBAPI 2.0 tests.
'''

__author__ = 'Miguel Ojeda'
__copyright__ = 'Copyright 2013, CERN'
__credits__ = ['Miguel Ojeda']
__license__ = 'Unknown'
__maintainer__ = 'Miguel Ojeda'
__email__ = 'mojedasa@cern.ch'


import unittest
import datetime

import pytz

import frontier


def connect():
    return frontier.connect('http://cmsfrontier.cern.ch:8000/FrontierPrep')


class TestFrontierConnections(unittest.TestCase):
    '''Tests on connections/cursors, which need to be set up manually.
    '''

    def test_closed_connection(self):
        connection = connect()
        connection.close()
        self.assertRaises(frontier.InterfaceError, connection.close)
        self.assertRaises(frontier.InterfaceError, connection.commit)
        self.assertRaises(frontier.InterfaceError, connection.cursor)

    def _test_cursor(self, c):
        self.assertRaises(frontier.InterfaceError, lambda: c.description)
        self.assertRaises(frontier.InterfaceError, lambda: c.rowcount)
        self.assertRaises(frontier.InterfaceError, c.close)
        self.assertRaises(frontier.InterfaceError, c.execute, 'select 1 from dual')
        self.assertRaises(frontier.InterfaceError, c.executemany, 'select 1 from dual', [1, 2, 3])
        self.assertRaises(frontier.InterfaceError, c.fetchone)
        self.assertRaises(frontier.InterfaceError, c.fetchmany)
        self.assertRaises(frontier.InterfaceError, c.fetchall)
        self.assertRaises(frontier.InterfaceError, c.setinputsizes, [1, 2, 3])
        self.assertRaises(frontier.InterfaceError, c.setoutputsize, 1)
        self.assertRaises(frontier.InterfaceError, lambda: c.connection)

    def test_closed_cursor(self):
        connection = connect()
        c = connection.cursor()
        c.close()
        self._test_cursor(c)
        connection.close()

    def test_cursor_on_closed_connection(self):
        connection = connect()
        c = connection.cursor()
        connection.close()
        self._test_cursor(c)

    def test_interleave_cursors(self):
        '''Simulates simple interleaved calls to different cursors based on
        the same connection, to test whether cursors are able to properly hold
        several an independent result set.
        '''

        connection = connect()

        c1 = connection.cursor()

        # c1 executing query before c2, c3, c4 are created
        c1.execute('select 1 from dual')
        c2 = connection.cursor()
        c3 = connection.cursor()
        c4 = connection.cursor()

        # c2 executing and fetching before c1 fetches result
        c2.execute('select 2 from dual')
        self.assertEqual(c1.fetchall()[0][0], 1)

        # c3 executing query after c2
        c3.execute('select 3 from dual')

        # c4 executing wrong query after c2
        self.assertRaises(frontier.ProgrammingError, c4.execute, 'badselect 4 from dual')

        # c3 fetching results before c2
        self.assertEqual(c3.fetchall()[0][0], 3)

        # c5 created, executes and fetches in-between
        c5 = connection.cursor()
        c5.execute('select 5 from dual')
        self.assertEqual(c5.fetchall()[0][0], 5)
        c5.close()

        # c4 (wrong one) closes
        c4.close()

        # c2 finally fetching results
        self.assertEqual(c2.fetchall()[0][0], 2)

        # cursors randomly close
        c3.close()
        c1.close()
        c2.close()

        connection.close()

    def test_interleave_connections(self):
        '''Simulates simple interleaved calls to different connections,
        to test whether multiple connections work properly in parallel.

        Similar to (superset of) test_interleave_cursors().
        '''

        connection1 = frontier.connect('http://cmsfrontier.cern.ch:8000/FrontierPrep')
        c11 = connection1.cursor()
        c12 = connection1.cursor()
        connection2 = frontier.connect('http://cmsfrontier.cern.ch:8000/FrontierInt')
        c21 = connection2.cursor()
        c11.execute('select 1, 1 from dual')
        c22 = connection2.cursor()
        c21.execute('select 2, 1 from dual')
        connection3 = frontier.connect('http://cmsfrontier3.cern.ch:8000/FrontierInt')
        c31 = connection3.cursor()
        c31.execute('select 3, 1 from dual')
        c32 = connection3.cursor()
        self.assertEqual(c11.fetchall()[0], [1, 1])
        self.assertRaises(frontier.ProgrammingError, c32.execute, 'select * from CMS_COND_RUN_INFO.ORA_C_FILLINFO where rownum <= 3')
        self.assertEqual(c21.fetchall()[0], [2, 1])
        c11.close()
        self.assertRaises(frontier.InterfaceError, c11.close)
        c22.execute('select 2, 2 from dual')
        connection1.close()
        self.assertRaises(frontier.InterfaceError, c12.close)
        c32.execute('select 3, 2 from dual')
        self.assertEqual(c32.fetchall()[0], [3, 2])
        self.assertEqual(c31.fetchall()[0], [3, 1])
        c31.close()
        c32.close()
        c22.close()
        connection3.close()
        c21.close()
        connection2.close()


class TestFrontier(unittest.TestCase):
    '''Normal tests -- one cursor assigned per test, on the same connection.
    '''

    # Connection kept for all tests, new cursor per test
    connection = connect()

    def setUp(self):
        self.c = self.connection.cursor()

    def tearDown(self):
        self.c.close()

    def assertDescriptionEqual(self, description):
        self.assertEqual([(x[0], x[1]) for x in self.c.description], description)

    def test_empty(self):
        query = '''
            select 1
            from dual
            where 1 = 0
        '''

        self.c.execute(query)
        self.assertDescriptionEqual([
            ('1', 'NUMBER'),
        ])
        self.assertEqual(self.c.fetchone(), None)

        self.c.execute(query)
        self.assertDescriptionEqual([
            ('1', 'NUMBER'),
        ])
        self.assertEqual(self.c.fetchmany(0), [])
        self.assertEqual(self.c.fetchmany(1), [])
        self.assertEqual(self.c.fetchmany(2), [])

        self.c.execute(query)
        self.assertDescriptionEqual([
            ('1', 'NUMBER'),
        ])
        self.assertEqual(self.c.fetchall(), [])

    def test_types(self):
        self.c.execute('''
            select
                NULL,
                'foo', CHR(124),
                124, 100*100*100*100*100, 1.1,
                current_date, current_timestamp,
                rowid, rownum
            from dual
        ''')

        rows = self.c.fetchall()
        self.assertEqual(len(rows), 1)
        self.assertEqual(self.c.rowcount, 1)

        self.assertDescriptionEqual([
            ('NULL', 'VARCHAR2'),
            ("'FOO'", 'CHAR'),
            ('CHR(124)', 'VARCHAR2'),
            ('124', 'NUMBER'),
            ('100*100*100*100*100', 'NUMBER'),
            ('1.1', 'NUMBER'),
            ('CURRENT_DATE', 'DATE'),
            ('CURRENT_TIMESTAMP', 'TIMESTAMP WITH TIME ZONE'),
            ('ROWID', 'ROWID'),
            ('ROWNUM', 'NUMBER'),
        ])

        (null, s, c, i, l, f, d, t, rowid, rownum) = rows[0]
        self.assertEqual(null, None)
        self.assertEqual(s, 'foo')
        self.assertEqual(c, '|')
        self.assertEqual(i, 124)
        self.assertEqual(l, 100*100*100*100*100)
        self.assertTrue(abs(f - 1.1) < 0.0001)
        now = datetime.datetime.now()
        self.assertTrue((now - d).seconds < 60 * 30) # difference less than 30 minutes
        t = t.astimezone(pytz.utc).replace(tzinfo = None) # make naive timestamp based on UTC
        now = datetime.datetime.utcnow()
        self.assertTrue((now - t).seconds < 60 * 30) # difference less than 30 minutes
        self.assertTrue(isinstance(rowid, str)) # Frontier returns them as strings
        self.assertTrue(len(rowid) > 4) # looks like currently (11g) they are 18 bytes long
        self.assertEqual(rownum, 1)

    def test_basic_query(self):
        self.c.execute('''
            with m as (
                select 1 a, 2 b, 3 c
                from dual
                union
                select 4 a, 5 b, 6 c
                from dual
                union
                select 7 a, 8 b, 9 c
                from dual
            )
            select sum(a) + sum(b) + sum(c) result
            from m
        ''')
        rows = self.c.fetchall()
        self.assertEqual(len(rows), 1)
        self.assertEqual(self.c.rowcount, 1)

        self.assertDescriptionEqual([
            ('RESULT', 'NUMBER'),
        ])

        self.assertTrue(rows[0][0], 1 + 2 + 3 + 4 + 5 + 6 + 7 + 8 + 9)

    def test_bind_variables(self):
        # Repeated variable names, underscore in variable names.
        query = '''
            with m as (
                select 1 a, 2 b, 3 c, 'hello' d
                from dual
                union
                select 4 a, 5 b, 6 c, 'foo' d
                from dual
                union
                select 7 a, 8 b, 9 c, 'bar' d
                from dual
                union
                select 10 a, 11 b, 12 c, 'bye' d
                from dual
            )
            select *
            from m
            where a > :a
                and b <= :shared
                and c <= :shared
                and d like :aA_2
        '''

        # Parameters: Sequence (list)
        self.c.execute(query, [1, 10, 10, 'f%'])

        rows = self.c.fetchall()
        self.assertEqual(len(rows), 1)
        self.assertEqual(self.c.rowcount, 1)

        self.assertDescriptionEqual([
            ('A', 'NUMBER'),
            ('B', 'NUMBER'),
            ('C', 'NUMBER'),
            ('D', 'VARCHAR2'),
        ])

        self.assertTrue(rows[0], (4, 5, 6, 'foo'))

        # Parameters: Sequence (tuple)
        self.c.execute(query, (1, 10, 10, 'f%'))

        rows = self.c.fetchall()
        self.assertEqual(len(rows), 1)
        self.assertEqual(self.c.rowcount, 1)

        self.assertDescriptionEqual([
            ('A', 'NUMBER'),
            ('B', 'NUMBER'),
            ('C', 'NUMBER'),
            ('D', 'VARCHAR2'),
        ])

        self.assertTrue(rows[0], (4, 5, 6, 'foo'))

        # Parameters: Sequence with wrong length
        self.assertRaises(frontier.ProgrammingError, self.c.execute, query, [])
        self.assertRaises(frontier.ProgrammingError, self.c.execute, query, [1])
        self.assertRaises(frontier.ProgrammingError, self.c.execute, query, [1, 2])
        self.assertRaises(frontier.ProgrammingError, self.c.execute, query, [1, 2, 3]) # one less
        self.assertRaises(frontier.ProgrammingError, self.c.execute, query, [1, 2, 3, 4, 5]) # one more
        self.assertRaises(frontier.ProgrammingError, self.c.execute, query, ())
        self.assertRaises(frontier.ProgrammingError, self.c.execute, query, (1))
        self.assertRaises(frontier.ProgrammingError, self.c.execute, query, (1, 2))
        self.assertRaises(frontier.ProgrammingError, self.c.execute, query, (1, 2, 3)) # one less
        self.assertRaises(frontier.ProgrammingError, self.c.execute, query, (1, 2, 3, 4, 5)) # one more

        # Parameters: Mapping (dict)
        self.c.execute(query, {
            'a': 1,
            'aA_2': 'f%',
            'shared': 10,
        })

        rows = self.c.fetchall()
        self.assertEqual(len(rows), 1)
        self.assertEqual(self.c.rowcount, 1)

        self.assertDescriptionEqual([
            ('A', 'NUMBER'),
            ('B', 'NUMBER'),
            ('C', 'NUMBER'),
            ('D', 'VARCHAR2'),
        ])

        self.assertTrue(rows[0], (4, 5, 6, 'foo'))

        # Parameters: Not provided
        self.assertRaises(frontier.ProgrammingError, self.c.execute, query)

        # Parameters: Unknown parameters type (i.e. not an instance of Sequence nor Mapping)
        self.assertRaises(frontier.ProgrammingError, self.c.execute, query, set([1, 2, 3, 4]))

        # Parameters: Unknown parameter type
        self.assertRaises(frontier.ProgrammingError, self.c.execute, query, (set([1, 2, 3, 4]), ))

        # Parameters: Colon in stringified parameter
        self.assertRaises(frontier.ProgrammingError, self.c.execute, query, [1, 'foo:bar', 3, 4])

    def test_real_query(self):
        '''Requires a connection to CMS' Frontier.
        '''

        query = '''
            select *
            from CMS_COND_RUN_INFO.ORA_C_FILLINFO
            where rownum <= 3
        '''

        self.c.execute(query)
        self.assertTrue(self.c.fetchone() is not None)
        self.assertTrue(self.c.fetchone() is not None)
        self.assertTrue(self.c.fetchone() is not None)
        self.assertTrue(self.c.fetchone() is None) # no more rows

        self.c.execute(query)
        self.assertEqual(len(self.c.fetchmany(1)), 1) # fetch 1
        self.assertEqual(len(self.c.fetchmany(2)), 2) # fetch 2 more, total 3
        self.assertEqual(len(self.c.fetchmany(1)), 0) # no more rows

        self.c.execute(query)
        rows = self.c.fetchall()
        self.assertEqual(len(rows), 3)
        for i in range(3):
            self.assertEqual(type(rows[i][0]), int)
            self.assertEqual(type(rows[i][1]), int)
            self.assertEqual(type(rows[i][2]), float)
            self.assertEqual(type(rows[i][3]), buffer)

    def test_no_result(self):
        self.assertRaises(frontier.InterfaceError, self.c.fetchone)
        self.assertRaises(frontier.InterfaceError, self.c.fetchmany, 0)
        self.assertRaises(frontier.InterfaceError, self.c.fetchmany, 1)
        self.assertRaises(frontier.InterfaceError, self.c.fetchmany, 2)
        self.assertRaises(frontier.InterfaceError, self.c.fetchall)

    def test_syntax_error(self):
        self.assertRaises(frontier.ProgrammingError, self.c.execute, '''
            badsyntax 1
            from dual
        ''')

    def test_missing_table(self):
        self.assertRaises(frontier.ProgrammingError, self.c.execute, '''
            select 1
            from badtable
        ''')

    def test_properties(self):
        # Before running a query, properties must be reset
        self.assertEqual(self.c.description, None)
        self.assertEqual(self.c.rowcount, -1)
        self.assertRaises(frontier.InterfaceError, self.c.fetchall)

        # Run a simple, correct query
        self.c.execute('''
            select 1 as a
            from dual
        ''')
        self.assertEqual(self.c.fetchall()[0][0], 1)
        self.assertDescriptionEqual([
            ('A', 'NUMBER')
        ])
        self.assertEqual(self.c.rowcount, 1)

        # Now run a wrong query
        self.assertRaises(frontier.ProgrammingError, self.c.execute, '''
            select 1
            from badtable
        ''')

        # Properties should be reset
        self.assertEqual(self.c.description, None)
        self.assertEqual(self.c.rowcount, -1)
        self.assertRaises(frontier.InterfaceError, self.c.fetchall)


if __name__ == '__main__':
    unittest.main()

