package gov.fnal.frontier;

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

    public void testConstructor() {
	try {
	    new Command("type","bah!");
	} catch (CommandException e) {}
    }

    public void testConstructorCommandException() {
	try {
	    new Command("Mr McGee","and sons");
	    fail("Should Raise CommandException");
	} catch (CommandException e) {}
    }
    
    public void testIsUniversalQueryCommand() {
	try {
	    Command command = new Command("type","bah!");
	    if (! command.isUniversalQueryCommand())
		fail("This Command should be a Universal Query Command");
	} catch (CommandException e) {}
    }

    public void testIsAdminCommand() {
	try {
	    Command command = new Command("admin","bah!");
	    if (! command.isAdminCommand())
		fail("It should be a Admin Command");
	} catch (CommandException e) {}
    }

    public void testContainsKey() {
	try {
	    Command command = new Command("type","bah!");
	    if (! command.containsKey("type"))
		fail("This should contain the key 'type'.");
	} catch (CommandException e) {}
    }

    public void testGet() {
	try {
	    Command command = new Command("type","bah!");
	    if (command.get("type") != "bah!")
		fail("get did not return the correct value");
	} catch (CommandException e) {
	} catch (NullPointerException e) {
	    fail("Internal test error.");
	}
    }   
	
    public void testPut() {
	try {
	    Command command = new Command("type","bah!");
	    command.put("zigBlat","dunno");
	    if (command.get("zigBlat") != "dunno")
		fail("put did not insert the value");
	} catch (CommandException e) {
	} catch (NullPointerException e) {
	    fail("Internal test error.");
	}
    }   
	

	
}
