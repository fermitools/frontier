package gov.fnal.frontier;

import java.util.*;
import javax.naming.*;

import java.lang.Exception.*;

/**
 * Contains all user suppled data required to execute a single user request.  
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

    public Command(String key, String value) throws CommandException {
	if (key == "type") {
	    commandType = univQueryCom;
	    put(key,value);
	}
	else if (key == "admin") {
		commandType = adminCom;
		put(key,value);
	}
	else
	    throw new CommandException("Unrecognized command type " + commandType);
    }

    public boolean isUniversalQueryCommand() {
	if (commandType == univQueryCom) 
	    return true;
	return false;
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

    public void put(String key, String value) throws NullPointerException {
	commandDict.put(key,value);
    }

    public int size() {
	return commandDict.size();
    }

}
