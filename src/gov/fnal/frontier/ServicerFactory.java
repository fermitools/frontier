package gov.fnal.frontier;

import java.io.BufferedInputStream;
import java.sql.Connection;
import java.sql.Statement;
import java.sql.ResultSet;
import java.sql.Blob;
import java.sql.SQLException;

/**
 * This class is repsonsible for creating a @link(Servicer) object of the
 * approprite subclass and returning it as Servicer to the caller.  Data for
 * determining the appropriate sub-class and for creating the object must
 * be stored int he table frontier_description.  This table must accesable
 * via the scheam the application has access to. Currenlty this method can handle
 * data stored in XML format.  It will be extended to support jar files.
 *
 * @author Stephen P. White
 * @version $Revision
 */
public class ServicerFactory {

    private DbConnectionMgr connMgr = DbConnectionMgr.getDbConnectionMgr();
    private XmlLoader xmlLoader = new XmlLoader();

    /**
     * Private Internal class.
     */
    private class SFDataResult {
        public String type = null;
        public BufferedInputStream data = null;

        /**
         * Constructor for a private class.
         * @param aType String identifes what the data returned is.
         * @param aData BufferedInputStream data required for creating the {@link Servicer}
         * sub-class.
         */
        SFDataResult(String aType, BufferedInputStream aData) {
            type = aType;
            data = aData;
        }
    }

    /**
     * Constructor.
     */
    public ServicerFactory() {}

    /**
     * This method oversees the construction of the Servicer sub-class.
     * @param className String is the class to create a {@link Servicer} for.
     * @param classVersion String is a specific version o the class to create.
     * @throws ServicerFactoryException
     * @return Servicer
     */
    public Servicer load(String className, String classVersion) throws
        ServicerFactoryException {
        Servicer servicer = null;

        /** @todo Add a cache for the Descriptor objects **/

        SFDataResult result = getData(className, classVersion);
        try {
            if (result.type.compareToIgnoreCase("xml") == 0) {
                XmlDescriptor descriptor = (XmlDescriptor) xmlLoader.load(
                    className, classVersion, result.data);
                servicer = new XmlServicer(descriptor);
            } else if (result.type.compareToIgnoreCase("jar") == 0) {
                throw new ServicerFactoryException("Jar files are not yet supported");
            } else {
                throw new ServicerFactoryException("Received an unknown type of " + result.type);
            }
        } catch (LoaderException e) {
            throw new ServicerFactoryException(e.getMessage());
        }
        return servicer;
    }

    /**
     * Obtains the data necessary for instanating the {@link Servicer} sub-class
     * from the frontier_descriptor table.
     * @param className String name of the class to obtain the data for.
     * @param classVersion String identifies a specific series of the class.
     * @throws ServicerFactoryException
     * @return SFDataResult holds a description of the type of data obtained and
     * the data itself.
     */
    private SFDataResult getData(String className, String classVersion) throws
        ServicerFactoryException {
        SFDataResult result = null;
        String query = "select xsd_type,xsd_data from frontier_descriptor where name = '"
                       + className + "'" + " and version = '" + classVersion + "'";
        Connection connection = null;
        Statement stmt = null;
        ResultSet rs = null;
        try {
            connection = connMgr.acquire();
            stmt = connection.createStatement();
            rs = stmt.executeQuery(query);
            long recordCnt = 0;
            while (rs.next()) {
                if (recordCnt > 0)
                    throw new ServicerFactoryException("Multiple rows obtained for query. "
                                                       + query);
                Blob blob = rs.getBlob(2);
                result = new SFDataResult(rs.getString(1),
                                          new BufferedInputStream(blob.getBinaryStream()));
                recordCnt++;
            }
            if (recordCnt == 0) {
                throw new ServicerFactoryException(
                    "No rows obtained for query. " + query);
            }
        } catch (DbConnectionMgrException e) {
            throw new ServicerFactoryException(e.getMessage());
        } catch (SQLException e) {
            throw new ServicerFactoryException(e.getMessage());
        } finally {
            try {
                if (rs != null)
                    rs.close();
            } catch (SQLException e) {}
            try {
                if (stmt != null)
                    stmt.close();
            } catch (SQLException e) {}
            try {
                connMgr.release(connection);
            } catch (DbConnectionMgrException e) {}
        }
        return result;
    }

}
