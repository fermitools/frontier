package gov.fnal.frontier;

/**
 * $Id$
 * $Author$
 * $Date$
 * $Revision$
 */

public class ClassLoader {

    ClassLoader() {}

    public Servicer load(String className, String classVersion) 
	throws ClassLoaderException {
	
	// hack test
	if (classVersion.compareTo("0")==0) {
	    String message = "The requested class/version was not found. className: ";
	    message += className + " version: " + classVersion;
	    throw new ClassLoaderException(message);
	}
	return new Servicer();
    }
}