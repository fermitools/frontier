package gov.fnal.frontier;

import java.util.Hashtable;
import java.util.ArrayList;
import java.util.StringTokenizer;

/**
 * This class is responsible for parsing a string into its a set of Commands.
 * The string is expected to be a URL which conforms to Frontier specifications.
 * Once parsed an ordered list of {@link Command}s is created.
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class CommandParser {

    private Hashtable majorKeys = new Hashtable();

    /**
     * Constructor
     */
    public CommandParser() {
        /* Add the major keyes into the table
           assign each key to a command type  that
           relates to a set method in the Command class.
         */
        majorKeys.put("type", "query");
	majorKeys.put("meta", "metainfo");
        majorKeys.put("junk", "admin"); // just to test, replace with real command
    }

    /**
     * Manages the parsing of a URL into its respective {@link Command}s
     * @param urlData String Frontier complient URL to parse.
     * @throws CommandParserException if missing or null data is encountered in
     * the URL.
     * @return ArrayList
     */
    public ArrayList parse(String urlData) throws CommandParserException {

        ArrayList commandList = new ArrayList();
        if(urlData != null) {
            StringTokenizer urlComponets = new StringTokenizer(urlData, "&");
            Command command = null;
            while(urlComponets.hasMoreTokens()) {
                String componet = urlComponets.nextToken();
                String[] keyValue = getKeyValue(componet);
                String key = keyValue[0];
                String value = keyValue[1];
                if(value == null)
                    throw new CommandParserValueException("Missing a value with key: " + key);
                String keyType = (String) majorKeys.get(key);
                if(keyType != null) {
                    if(command != null)
                        commandList.add(command);
                    command = new Command(key, value);
                    setCommandType(command, keyType);
                } else if(command == null) {
                    String msg = "Request was started without a known major keyword, ";
                    msg += "keyword received: " + key;
                    throw new CommandParserException(msg);
                } else
                    command.put(key, value);
            }
            if(command != null)
                commandList.add(command);
        }
        return commandList;
    }

    /**
     * Internal method which sets the {@link Command} to a specific type.
     * @param command Command The instance of {@link Command} to set.
     * @param keyType String The major key for determining the command type.
     * @throws CommandParserException
     */
    private void setCommandType(Command command, String keyType) throws CommandParserException {
        if(keyType == "query")
            command.setUniversalQueryCommand();
        else if(keyType == "metainfo")
            command.setMetaQueryCommand();        
	else if(keyType == "admin")
            command.setAdminCommand();
        else
            throw new CommandParserException("Internal Error, Unknown command type: " + keyType);
    }

    /**
     * Parses a URL key/value combination into its respective componets.
     * @param keyValue String Key/value pair to parse.
     * @return String[] The key is returned in position 0 and the value in
     * position 1.
     */
    private String[] getKeyValue(String keyValue) {
        StringTokenizer componets = new StringTokenizer(keyValue, "=");
        String value = null; // Allow flags, not just key/value pairs
        String key = componets.nextToken();
        if(componets.hasMoreTokens())
            value = componets.nextToken();
        return new String[] {key, value};
    }

}
