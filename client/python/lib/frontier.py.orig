'''Frontier binding for Python.

This module follows the Python Database API Specification v2.0, PEP 249 [1],
and can be easily used as a replacement for cx_Oracle where applicable.

Requires pytz if any query returns timezone-aware timestamps from Oracle.

If you are already using SQLAlchemy, take a look at the SQLAlchemy dialect
based on this module.

[1] http://www.python.org/dev/peps/pep-0249/
'''

__author__ = 'Miguel Ojeda'
__copyright__ = 'Copyright 2013, CERN'
__credits__ = ['Miguel Ojeda']
__license__ = 'Unknown'
__maintainer__ = 'Miguel Ojeda'
__email__ = 'mojedasa@cern.ch'


import datetime
import time
import re
import collections
import logging
import sys

try:
    import pytz
except ImportError:
    pytz = None

import frontier_client


logger = logging.getLogger(__name__)

# Set initial level to WARN.  This so that log statements don't occur in
# the absense of explicit logging being enabled.
if logger.level == logging.NOTSET:
    logger.setLevel(logging.WARN)


def connect(server_url = None, proxy_url = None):
    '''Returns a Connection object.

    The connection will be configured according to:

        1. The environment variables defined by Frontier.
        2. The passed URLs.

    Note that the server_url (and its environment variable counterpart) can be
    a simple HTTP URL or a complex Frontier configuration URL that sets many
    configuration values, e.g.

        frontier://(k1=v1)...(kn=vn)/database

    While this complex URLs are the most used ones, users normally do not
    directly use them. Instead, they will use an experiment-provided
    application or service that builds them given a shorthand name.
    For instance, in CMS, a user could run:

        cmsGetFnConnect frontier://FrontierPrep

    Since this module should remain experiment-agnostic, users need to supply
    the proper way to fetch the complex URLs.
    '''

    return Connection(server_url, proxy_url)


apilevel = '2.0'
threadsafety = 1
paramstyle = 'named'

_paramchar = '?'
_separatorchar = ':'


class Error(Exception):
    pass

class Warning(Exception):
    pass

class InterfaceError(Error):
    pass

class DatabaseError(Error):
    pass

class InternalError(DatabaseError):
    pass

class OperationalError(DatabaseError):
    pass

class ProgrammingError(DatabaseError):
    pass

class IntegrityError(DatabaseError):
    pass

class DataError(DatabaseError):
    pass

class NotSupportedError(DatabaseError):
    pass


class Connection(object):
    def __init__(self, server_url=None, proxy_url=None):
        self._server_url = server_url
        self._proxy_url = proxy_url
        self._closed = False
        self._channel = frontier_client.frontier_createChannel(self._server_url, self._proxy_url)

    def _check_closed(self):
        if self._closed:
            raise InterfaceError('The connection is already closed.')

    def close(self):
        self._check_closed()
        self._closed = True
        frontier_client.frontier_closeChannel(self._channel)

    def commit(self):
        self._check_closed()

    def cursor(self):
        self._check_closed()
        return Cursor(self)


def _stringify(parameter):
    if parameter is None:
        return 'NULL'

    elif isinstance(parameter, bool):
        if parameter:
            return 'TRUE'
        else:
            return 'FALSE'

    elif isinstance(parameter, str):
        return parameter

    elif isinstance(parameter, int) or isinstance(parameter, long) or isinstance(parameter, float):
        return str(parameter)
    elif isinstance(parameter, datetime.datetime):
        return "TO_TIMESTAMP('"+str(parameter.strftime("%Y-%m-%d %H %M %S"))+"','YYYY-MM-DD HH24 MI SS')"

    raise ProgrammingError('Unsupported parameter type (%s).' % type(parameter))


def _parse_timestamp(timestamp):
    '''Most of the timestamps we have seen follow the first format.
    However, we found an instance where a TIMESTAMP was lacking
    the subsecond precision. Therefore, try both.

    In addition, DATE uses the second format as far as we have seen.

    This function should cover both cases.
    '''

    # XXX: We should enforce a protocol/format on times/dates.

    # XXX: What about only DATES without subday precision?

    try:
        return datetime.datetime.strptime(timestamp, '%Y-%m-%d %H:%M:%S.%f')
    except ValueError:
        return datetime.datetime.strptime(timestamp, '%Y-%m-%d %H:%M:%S')


class Cursor(object):
    arraysize = 1

    def __init__(self, connection):
        self._connection = connection
        self._closed = False
        self._reset()

    def _reset(self):
        self._description = None
        self._rowcount = -1
        self._result = None
        self._rowpos = None

    def _check_closed(self):
        if self._closed:
            raise InterfaceError('The cursor is already closed.')

        # Check as well that the connection of this cursor is not closed
        self._connection._check_closed()

    @property
    def description(self):
        # name, type, display_size, internal_size, precision, scale, null_ok
        self._check_closed()
        return self._description

    @property
    def rowcount(self):
        self._check_closed()
        return self._rowcount

    def close(self):
        self._check_closed()
        self._closed = True

    def execute(self, operation, parameters = None):
        self._check_closed()

        # Find the variables, e.g. [':name', ':1', ':ROWNUM_1']
        # XXX: We would need a proper Oracle SQL parser here. At the moment,
        # users are not be able to use the ':' character in a string literal
        # if followed by letters/numbers/underscore. A workaround for them
        # is to use "select ':' || 'abc'" instead of "select ':abc'".
        if isinstance(operation, bytes):
            operation = operation.decode()
            sparameters = {}
            if parameters:
                for p in parameters:
                    sparameters[p.decode()] = parameters[p]
                parameters = sparameters
        variables = re.findall(':[a-zA-Z-0-9_]+', operation)
        logger.debug('Variables = %s', variables)

        # Build the SQL and parameter list as the Frontier server expects:
        #
        # ''' Support bind variables in queries, with a question mark where
        #     each variable is to be inserted.  The values for the variables
        #     must be appended to the query, separated by colons (:). '''
        final_parameters = []

        if parameters is None:
            if len(variables) != 0:
                raise ProgrammingError('Bind variables used, but parameters were not provided.')

        elif isinstance(parameters, collections.Sequence):
            if len(parameters) != len(variables):
                raise ProgrammingError('Different length on parameters (%s) and bind variables list (%s) while using positional parameters.' % (len(parameters), len(variables)))

            for i in range(len(variables)):
                operation = operation.replace(variables[i], _paramchar, 1)

            final_parameters = parameters

        elif isinstance(parameters, collections.Mapping):
            for variable in variables:
                if isinstance(parameters[variable[1:]],datetime.datetime):
                    operation = operation.replace(variable,_stringify(parameters[variable[1:]]))
                else:
                    operation = operation.replace(variable, _paramchar, 1)
                    final_parameters.append(parameters[variable[1:]])

        else:
            raise ProgrammingError('Unsupported parameters type (%s).' % type(parameters))

        final_parameters = [_stringify(x) for x in final_parameters]
    
        # XXX: Check there are no colons (:) in any value (it is not supported
        # by the Frontier server yet -- there is no way to escape them)
        for parameter in final_parameters:
            if _separatorchar in parameter:
                raise ProgrammingError("Unsupported character '%s' in parameter (%s)." % (_separatorchar, parameter))

        logger.info('%s', operation)
        logger.info('%s', final_parameters)

        operation = _separatorchar.join([operation] + final_parameters)
        logger.debug('Query to Frontier = %s', operation)

        # Build the URI
        uri = 'Frontier/type=frontier_request:1:DEFAULT&encoding=BLOBzip5&p1=%s' % \
              frontier_client.fn_gzip_str2urlenc(operation, decode=sys.version_info[0]>2)

        try:
            frontier_client.frontier_getRawData(self._connection._channel, uri)
        except frontier_client.FrontierClientError as e:
            self._reset()
            raise ProgrammingError('Error while fetching data: %s' % e)

        oldresult = self._result
        self._result = frontier_client.frontierRSBlob_open(self._connection._channel, self._result, 1)
        if oldresult:
            frontier_client.frontierRSBlob_close(oldresult)

        self._rowcount = frontier_client.frontierRSBlob_getRecNum(self._result)

        # self.rowcount is correct; however, frontier returns one more record,
        # the first one, with the description (name, type) of the columns
        # for the real rows. This is what we will use to fill self.description
        row = self._fetchone(parse = False)
        self._description = []
        for i in range(0, len(row), 2):
            # name, type, display_size, internal_size, precision, scale, null_ok
            self._description.append((row[i], row[i+1], None, None, None, None, None))

        if logger.isEnabledFor(logging.DEBUG):
            logger.debug('Col %s', [x[0] for x in self._description])

        self._rowpos = 0

    def executemany(self, operation, seq_of_parameters):
        self._check_closed()
        for parameters in seq_of_parameters:
            self.execute(operation, parameters)

    def _fetchone(self, parse):
        row = []

        i = -1
        while True:
            i += 1

            data_type = frontier_client.frontierRSBlob_getByte(self._result)

            # End
            if data_type == frontier_client.BLOB_TYPE_EOR:
                break

            # NULL
            if data_type & frontier_client.BLOB_TYPE_NONE:
                row.append(None)
                continue

            # Normal value. For the moment, Frontier *always* uses
            # frontier_client.BLOB_TYPE_ARRAY_BYTE (i.e. returns all values
            # as strings) and the client needs to parse the first record
            # (type information) to convert the value to the proper type.
            if data_type != frontier_client.BLOB_TYPE_ARRAY_BYTE:
                raise InternalError('Unexpected data type (%s).' % data_type)

            decode_value = False
            if sys.version_info[0]>2:
                if (not parse) or (not self._description[i][1] in ('BLOB', 'CLOB')):
                    decode_value = True
            value = frontier_client.frontierRSBlob_getByteArray(self._result, decode_value)

            if parse:
                columnType = self._description[i][1]

                # XXX: This is a bit hacky. Maybe this could be done in
                # in a cleaner way in the frontier_client C library.

                # startswith() is used since the type may be returned with
                # the size, e.g. NUMBER(3).
                if 'CHAR' in columnType or columnType == 'ROWID':
                    pass

                elif columnType.startswith('NUMBER'):
                    value = float(value)
                    if value.is_integer():
                        value = int(value)

                elif columnType.startswith('BINARY_FLOAT') or columnType.startswith('BINARY_DOUBLE'):
                    # CPython's float is C's double, so should be fine for both
                    # If not, should we use Python's decimal?
                    value = float(value)

                elif columnType in ('BLOB', 'CLOB'):
                    value = Binary(value)

                elif columnType == 'DATE':
                    value = _parse_timestamp(value)

                elif columnType.startswith('TIMESTAMP'):
                    if 'WITH TIME ZONE' in columnType:
                        if pytz is None:
                            raise InterfaceError('Query returned a timezone-aware timestamp, but the pytz module is not available.')

                        (timeString, timeZone) = value.rsplit(' ', 1)
                        value = pytz.timezone(timeZone).localize(_parse_timestamp(timeString))
                    elif 'WITH LOCAL TIMEZONE' in columnType:
                        raise NotSupportedError('Unsupported type (%s).' % columnType)
                    else:
                        value = _parse_timestamp(value)

                else:
                    raise NotSupportedError('Unsupported type (%s).' % columnType)

            row.append(value)

        return row

    def _check_result(self):
        if self._result is None:
            raise InterfaceError('Fetched from a cursor without executing a query first.')

    def fetchone(self):
        self._check_closed()
        self._check_result()

        if self._rowpos == self._rowcount:
            return None
        self._rowpos += 1

        row = self._fetchone(parse = True)
        if len(self._description) != len(row):
            raise InternalError('Row description length (%s) does not match number of columns in row (%s)' % (len(self._description), len(row)))

        logger.debug('Row %s', row)

        return row

    def fetchmany(self, size=arraysize):
        self._check_closed()
        self._check_result()

        result = []
        for n in range(size):
            row = self.fetchone()
            if row is None:
                break
            result.append(row)
        return result

    def fetchall(self):
        self._check_closed()
        self._check_result()

        result = []
        while True:
            row = self.fetchone()
            if row is None:
                break
            result.append(row)
        return result

    def setinputsizes(self, sizes):
        self._check_closed()

    def setoutputsize(self, size, column=None):
        self._check_closed()

    @property
    def connection(self):
        self._check_closed()
        return self._connection

# Not used. These objects are exposed in compliance to the DB API.
# psycopg2 exposes them but are not used. cx_Oracle exposes proper types,
# but fills cursor.description in a non-compliant way. sqlite3 does not even
# expose the type objects (but keeps Date, Time, et. al.). Therefore,
# we stick with the easiest and probably most useful way, which
# is just returning the string given by Frontier as the type of a column,
# e.g. 'VARCHAR2'.

Date = datetime.date

Time = datetime.time

Timestamp = datetime.datetime

def DateFromTicks(ticks):
    return Date(*time.localtime(ticks)[:3])

def TimeFromTicks(ticks):
    return Time(*time.localtime(ticks)[3:6])

def TimestampFromTicks(ticks):
    return Timestamp(*time.localtime(ticks)[:6])

Binary = bytearray

#class STRING(object):
#    pass

#class BINARY(object):
#    pass

#class NUMBER(object):
#    pass

#class DATETIME(object):
#    pass

#class ROWID(object):
#    pass

