package gov.fnal.frontier;

import java.util.*;
import javax.naming.*;

/**
 * This class is responsible for parsing a string into its "commands".  The string is expected to
 * the data from a URL.  Once parsed it creates an ordered list of Commands which hold all key/values 
 * of each command.
 *
 * $Id$
 * $Author$
 * $Date$
 * $$
 */

public class CommandParser {

    private Hashtable majorKeys = new Hashtable();
    
    public CommandParser() {
	/* Add the major keyes into the table 
	   assign each key to a command type  that 
	   relates to a set method in the Command class.
	*/
	majorKeys.put("type","query");
	majorKeys.put("junk","admin");  // just to test, replace with real command
    }

    public ArrayList parse(String urlData)  throws CommandParserException {

	ArrayList commandList = new ArrayList();
	if (urlData != null) {
	    StringTokenizer urlComponets = new StringTokenizer(urlData,"&");
	    Command command = null;
	    while (urlComponets.hasMoreTokens()) {
		String componet = urlComponets.nextToken();
		String [] keyValue = getKeyValue(componet);
		String key   = keyValue[0];
		String value = keyValue[1];
		if (value == null)
		    throw new CommandParserValueException("Missing a value with key: " + key);
		String keyType = (String) majorKeys.get(key);
		if (keyType != null) {
		    if (command != null)
			commandList.add(command);
		    command = new Command(key,value);
		    setCommandType(command,keyType);
		}
		else if (command == null) {
		    String msg = "Request was started without a known major keyword, ";
		    msg += "keyword received: " + key;
		    throw new CommandParserException(msg);
		}
		else
		    command.put(key,value);
	    }
	    if (command != null)
		commandList.add(command);
	}
	return commandList;
    }

    private void setCommandType(Command command,String keyType) throws CommandParserException {
	if (keyType == "query")
	    command.setUniversalQueryCommand();
	else if (keyType == "admin")
	    command.setAdminCommand();
	else
	    throw new CommandParserException("Internal Error, Unknown command type: " + keyType);
    }

    private String[] getKeyValue(String keyValue) {
	StringTokenizer componets = new StringTokenizer(keyValue,"=");
	String value = null;  // Allow flags, not just key/value pairs
	String key = componets.nextToken();
	if (componets.hasMoreTokens())
	    value = componets.nextToken();
	return new String[] {key,value};
    }
 
}

