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

    private static final String sql_pescalib=
     "select address,gain,sourcemean,sourcerms,sourcenorm,lasernorm,"+
     "linearity1,linearity2,linearity3,attenuation1,attenuation2,attenuation3 "+
     "from pescalib where cid = ? ";
    private static final String sql_calibrunlist=
     "select CALIB_RUN,CALIB_VERSION,PERIOD,DATA_STATUS from CALIBRUNLISTS WHERE CID = ? ";
    
    String tableName    = null;
    public Long   cid          = null;

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
	String sql="select * FROM  " + tableName + " where CID = ? ";
	if (tableName.compareToIgnoreCase("calibrunlists")==0)
	    sql=sql_calibrunlist;
	else if(tableName.compareToIgnoreCase("cmuchannel")==0)
	    sql=sql+" order by 1,2";
	else if(tableName.compareToIgnoreCase("pescalib")==0)
	    sql=sql_pescalib;
	return sql;
    }

    public long marshal(Encoder encoder,ResultSet resultSet) throws SQLException, Exception {
	ResultSetMetaData rsmd = resultSet.getMetaData();
	int    columnCnt = rsmd.getColumnCount();
	long recordCnt = 0;
	while (resultSet.next()) {
	    recordCnt += 1;
	    //System.out.println("columnCnt: " + columnCnt);
	    for (int cnt=1;cnt<=columnCnt;cnt++) {
		String columnType = rsmd.getColumnTypeName(cnt);
		String name = rsmd.getColumnName(cnt);
		//System.out.println("colCnt: " + cnt + " name: " + name + " columnType: " + columnType);
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

