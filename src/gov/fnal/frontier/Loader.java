package gov.fnal.frontier;

import java.io.InputStream;

/**
 * Generic class responsible for converting data into a {@link Descriptor} which
 * which provides detail on how the Servicer is to operate.
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class Loader {

    /**
     * Constructor.
     */
    Loader() {}

    /**
     * Generic method responsible for "loading" data into a {@link Descriptor} object
     * which identifies how a {@link Servicer} is to operate.
     * @param name String name of the object
     * @param version String version of the object
     * @param data InputStream data describing the object
     * @throws LoaderException
     * @return Descriptor
     */
    public Descriptor load(String name, String version, InputStream data) throws LoaderException {
        return null;
    }
}
