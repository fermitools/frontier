package gov.fnal.frontier;

import java.util.Hashtable;
import java.util.Enumeration;

/**
 * Contains the connents of a single command as supplied by the user.
 * Outside classes are allowed to remove commands from this class as they
 * processes them.
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class Command {

    private Hashtable commandDict = new Hashtable();

    private int commandType = 0; // Invalid command
    private final static int univQueryCom = 1;
    private final static int adminCom = 2;
    private final static int metaQueryCom = 3;

    /**
     * Constructor.
     * @param key String key - the major command keyword.
     * @param value String - the value of the major keyword.
     */
    public Command(String key, String value) {
        put(key, value);
    }

    /**
     * Returnes true if the instance is a query command.
     * @return boolean
     */
    public boolean isUniversalQueryCommand() {
        if(commandType == univQueryCom)
            return true;
        return false;
    }

    /**
     * Sets the instance to be a query command.
     */
    public void setUniversalQueryCommand() {
        commandType = univQueryCom;
    }
    
   public void setMetaQueryCommand(){commandType=metaQueryCom;}
   public boolean isMetaQueryCommand(){return commandType==metaQueryCom;}

    /**
     * Sets the instance to be an administration command.
     */
    public void setAdminCommand() {
        commandType = adminCom;
    }

    /**
     * Returns true if the instaces is an administration command.
     * @return boolean
     */
    public boolean isAdminCommand() {
        if(commandType == adminCom)
            return true;
        return false;
    }

    /**
     * Returns true if the requested key exists.
     * @param key String The value to perfom an existance check on.
     * @throws NullPointerException if the key is null
     * @return boolean
     */
    public boolean containsKey(String key) throws NullPointerException {
        return commandDict.containsKey(key);
    }

    /**
     * Returns the value of key.
     * @param key String The data to return the value for.
     * @throws NullPointerException if the key is null.
     * @return String
     */
    public String get(String key) throws NullPointerException {
        return(String) commandDict.get(key);
    }

    /**
     * Removes the key from this instances and returns it to the caller.
     * @param key String The key to remove and return.
     * @throws NullPointerException if the key is null
     * @return String
     */
    public String remove(String key) throws NullPointerException {
        return(String) commandDict.remove(key);
    }

    /**
     * Returns all the keys in the instance.  Each value in the Enumeration must
     * be cast to a String.
     * @return Enumeration
     */
    public Enumeration keys() {
        return commandDict.keys();
    }

    /**
     * Adds a key/value pair to this instance.
     * @param key String Tag to identify the data by.
     * @param value String Data to be saved.
     * @throws NullPointerException if the key or value is null.
     */
    public void put(String key, String value) throws NullPointerException {
        commandDict.put(key, value);
    }

    /**
     * Returns the number of key/value combinations in the instance.
     * @return int
     */
    public int size() {
        return commandDict.size();
    }

    /**
     * Ouputs contents of the instances as a String suitable for printing.
     * @return String
     */
    public String dump() {
        String dumpString = "Type: ";
        if(isUniversalQueryCommand())
            dumpString += "Universal Query Command";
        else if(isAdminCommand())
            dumpString += "Administration Command";
        else
            dumpString += "???? What am I ????";

        Enumeration keys = commandDict.keys();
        while(keys.hasMoreElements()) {
            String key = (String) keys.nextElement();
            dumpString += "\n" + "Key: " + key + " value: " + (String) commandDict.get(key);
        }
        dumpString += "\n";
        return dumpString;
    }
}
