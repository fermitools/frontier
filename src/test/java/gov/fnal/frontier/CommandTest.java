package gov.fnal.frontier;

import java.util.Enumeration;
import java.util.NoSuchElementException;

import gov.fnal.frontier.Command;
import junit.framework.TestCase;

public class CommandTest extends TestCase {

    public CommandTest(String name) {
        super(name);
    }

    public void setUp() {
        // Do nutton for now
    }

    public void tearDown() {
        // Do nutton for now
    }

    public void testIsUniversalQueryCommand() {
        Command command = new Command("type", "bah!");
        command.setUniversalQueryCommand();
        if(!command.isUniversalQueryCommand())
            fail("This Command should be a Universal Query Command");
    }

    public void testIsAdminCommand() {
        Command command = new Command("admin", "bah!");
        command.setAdminCommand();
        if(!command.isAdminCommand())
            fail("It should be a Admin Command");
    }

    public void testContainsKey() {
        Command command = new Command("type", "bah!");
        if(!command.containsKey("type"))
            fail("This should contain the key 'type'.");
    }

    public void testGet() {
        try {
            Command command = new Command("type", "bah!");
            if(command.get("type") != "bah!")
                fail("get did not return the correct value");
        } catch(NullPointerException e) {
            fail("Internal test error.");
        }
    }

    public void testKeys() {
        try {
            Command command = new Command("type", "bah!");
            command.put("aKey", "avalue");
            command.put("aKey2", "aValue");
            Enumeration aEnum = command.keys();
            aEnum.nextElement();
            aEnum.nextElement();
            aEnum.nextElement();
        } catch(NoSuchElementException e) {
            fail("Enumeration did not contain 3 objects!");
        } catch(NullPointerException e) {
            fail("Internal test error.");
        }
    }

    public void testPut() {
        try {
            Command command = new Command("type", "bah!");
            command.put("zigBlat", "dunno");
            if(command.get("zigBlat") != "dunno")
                fail("put did not insert the value");
        } catch(NullPointerException e) {
            fail("Internal test error.");
        }
    }

    public void testDump() {
        Command command = new Command("type", "bah!");
        command.setUniversalQueryCommand();
        System.out.println(command.dump());
    }

}
