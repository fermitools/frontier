package gov.fnal.frontier;

import java.util.*;
import javax.naming.*;

/**
 * Contains all user suppled data required to execute a single user request.  Outside classes
 * are allowed to remove commands from this class as they processes them.
 *
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 */

public class Command {

    private Hashtable commandDict = new Hashtable();
    private static String [] commandList = {"type"};

    private int commandType  = 0; // Invalid command
    private int univQueryCom = 1;
    private int adminCom     = 2;

    public Command(String key, String value) {
	put(key,value);
    }

    public boolean isUniversalQueryCommand() {
	if (commandType == univQueryCom) 
	    return true;
	return false;
    }
	
    public void setUniversalQueryCommand() {
	commandType = univQueryCom;
    }

    public void setAdminCommand() {
	commandType = adminCom;
    }

    public boolean isAdminCommand() {
	if (commandType == adminCom)
	    return true;
	return false;
    }

    public boolean containsKey(String key) throws NullPointerException {
	return commandDict.containsKey(key);
    }

    public String get(String key) throws NullPointerException {
	return (String) commandDict.get(key);
    }

    public String remove(String key) throws NullPointerException {
	return (String) commandDict.remove(key);
    }

    public Enumeration keys() {
	return commandDict.keys();
    }

    public void put(String key, String value) throws NullPointerException {
	commandDict.put(key,value);
    }

    public int size() {
	return commandDict.size();
    }

    public String dump() {
	String dumpString = "Type: ";
	if (isUniversalQueryCommand())
	    dumpString += "Universal Query Command";
	else if (isAdminCommand())
	    dumpString += "Administration Command";
	else
	    dumpString += "???? What am I ????";
	
	Enumeration keys = commandDict.keys();
	while (keys.hasMoreElements()) {
	    String key = (String) keys.nextElement();
	    dumpString += "\n" + "Key: " + key + " value: " + (String) commandDict.get(key);
	}
	dumpString += "\n";
	return dumpString;
    }
}
