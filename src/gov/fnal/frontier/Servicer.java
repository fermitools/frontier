package gov.fnal.frontier;

import java.util.*;
import java.sql.*;
import javax.sql.*;
import gov.fnal.frontier.codec.*;

/**
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 */

public class Servicer {

    String tableName    = null;
    Long   cid           = null;

    Servicer() {}

    public void validate(Command command) throws ServicerValidationException {

	String type = command.get("type");
	StringTokenizer typeComponets = new StringTokenizer(type,":");
	tableName    = typeComponets.nextToken();
	String cidKey    = command.get("cid");
	if (cidKey == null)
	    throw new ServicerValidationException("The key 'cid' was not provided.");
	try {
	    cid = new Long(cidKey);
	} catch (NumberFormatException e) {
	    throw new ServicerValidationException("Unable to convert value of cid to long. Value is: " + cidKey);
	}
    }
	
    public String getSql() {
	String sql = "select * FROM  " + tableName + " where CID = " + cid.longValue();
	if (tableName.compareToIgnoreCase("calibrunlists")==0)
	    sql = "select CALIB_RUN,CALIB_VERSION,PERIOD,DATA_STATUS from CALIBRUNLISTS WHERE CID = " + cid.longValue();
	return sql;
    }

    public long marshal(Encoder encoder,ResultSet resultSet) throws SQLException, Exception {
	ResultSetMetaData rsmd = resultSet.getMetaData();
	int    columnCnt = rsmd.getColumnCount();
	long recordCnt = 0;
	while (resultSet.next()) {
	    recordCnt += 1;
	    for (int cnt=1;cnt<=columnCnt;cnt++) {
		String columnType = rsmd.getColumnTypeName(cnt);
		if ( columnType == "RAW")
		    encoder.writeBytes(resultSet.getBytes(cnt));
		else if (columnType == "NUMBER")
		    if (rsmd.getScale(cnt)==0)
			encoder.writeLong(resultSet.getLong(cnt));
		    else
			encoder.writeDouble(resultSet.getDouble(cnt));
		else if (columnType == "VARCHAR2")
		    encoder.writeString(resultSet.getString(cnt));
		else
		    throw new Exception("Unknown columnType: " + columnType);
	    }
	}
	encoder.flush();
	return recordCnt;
    }
}

