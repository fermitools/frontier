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

    public void marshal(Encoder encoder,ResultSet sqlResults) throws ServicerException {

    }
}

