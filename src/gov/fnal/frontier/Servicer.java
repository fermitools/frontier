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
    public int param_num=1;
    public Command cmd=null;

    Servicer() {}

    /**
     * Validates that the data in the Command produces a
     * vaild SQL where clause.  As a side effect the SQL
     * select statement is built and loaded into the instance.
     *
     * @param command An instance of the Command class.
     * @exception ServicerValidationExeption Thrown if a valid
     * SQL statement cannot be created.
     * @return void.
     * 
     */
    public void validateAndLoad(Command command) throws ServicerValidationException {
	throw new ServicerValidationException("Servicer.validateAndLoad must be overridden.");
    }
	
    public void validate(Command command) throws ServicerValidationException {

        cmd=command;
	String type=command.get("type");
	StringTokenizer typeComponets = new StringTokenizer(type,":");
	tableName=typeComponets.nextToken();
    }
	
    public String getSql() {
	String sql="select * FROM  " + tableName + " where CID = ? ";
	if (tableName.compareToIgnoreCase("calibrunlists")==0)
	    sql=sql_calibrunlist;
	else if(tableName.compareToIgnoreCase("cmuchannel")==0)
	    sql=sql+" order by 1,2";
	else if(tableName.compareToIgnoreCase("pescalib")==0)
	    sql=sql_pescalib;
	else if(tableName.compareToIgnoreCase("frontier_get_cid_list")==0)
	    {
	     sql="select distinct cid from "+cmd.get("table_name")+" order by cid";
	     System.out.println("Get cid sql <"+sql+">");
	     param_num=0;
	    }
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

