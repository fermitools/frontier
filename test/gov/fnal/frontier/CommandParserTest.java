package gov.fnal.frontier;

import java.util.ArrayList;
import junit.framework.TestCase;

public class CommandParserTest extends TestCase {

    public CommandParserTest(String name) {
        super(name);
    }

    public void setUp() {
        // Do nutton for now
    }

    public void tearDown() {
        // Do nutton for now
    }

    /**
     * Test valid parameters for "type"
     */
    public void testParse1() {
        try {
            CommandParser parser = new CommandParser();
            String data = "&type=svxbeamposition:1&encoding=BLOB&cid=23457";
            ArrayList commandList = parser.parse(data);
            int count = commandList.size();
            if(count != 1)
                fail("Should be 1 item in the ArrayList, there is: " + count);
            Command command = (Command) commandList.get(0);
            if(!command.isUniversalQueryCommand())
                fail("Should be a query command, but it ain't!");
            String value = command.get("type");
            if(value.compareTo("svxbeamposition:1") != 0)
                fail("For type, returned the wrong value, value: " + value);
            value = command.get("encoding");
            if(value.compareTo("BLOB") != 0)
                fail("For encoding, returned the wrong value, value: " + value);
            value = command.get("cid");
            if(value.compareTo("23457") != 0)
                fail("For CID returned the wrong value, value: " + value);
        } catch(CommandParserException e) {
            fail("CommandParserException: " + e.getMessage());
        }
    }

    /**
     * Test valid parameters for "admin" command, includes a flag in this test.
     */
    public void testParse2() {
        try {
            CommandParser parser = new CommandParser();
            String data = "&junk=zippy&akey=avalue";
            ArrayList commandList = parser.parse(data);
            int count = commandList.size();
            if(count != 1)
                fail("Should be 1 item in the ArrayList, there is: " + count);
            Command command = (Command) commandList.get(0);
            if(!command.isAdminCommand())
                fail("Should be a admin command, but it ain't!");
            String value = command.get("junk");
            if(value.compareTo("zippy") != 0)
                fail("For type, returned the wrong value, value: " + value);

        } catch(CommandParserException e) {
            fail("CommandParserException: " + e.getMessage());
        }
    }

    /**
     * Test multiple valid parameters
     */
    public void testParse3() {
        try {
            CommandParser parser = new CommandParser();
            String data = "&type=svxbeampostion:1&encoding=BLOB&cid=23457";
            data += "&junk=zippy&go=away";
            ArrayList commandList = parser.parse(data);
            int count = commandList.size();
            if(count != 2)
                fail("Should be 2 item in the ArrayList, there is: " + count);
        } catch(CommandParserException e) {
            fail("CommandParserException: " + e.getMessage());
        }
    }

    /**
     * Test exception for key without a value.
     */
    public void testCommandParserValueException() {
        try {
            CommandParser parser = new CommandParser();
            String data = "&junk=zippy&dunno";
            ArrayList commandList = parser.parse(data);
            fail("Did not throw the CommandParserValueException!");
        } catch(CommandParserValueException e) {
        } catch(CommandParserException e) {
            fail("CommandParserException: " + e.getMessage());
        }
    }

}
