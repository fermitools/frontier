package gov.fnal.frontier;

import javax.servlet.ServletException;
import javax.servlet.ServletOutputStream;
import gov.fnal.frontier.codec.Encoder;
import gov.fnal.frontier.codec.BlobTypedEncoder;
import java.util.StringTokenizer;
import java.io.IOException;
import java.sql.Connection;
import java.sql.ResultSet;
import java.sql.PreparedStatement;
import java.sql.SQLException;

/**
 * This class controls the processing of a single query {@link Command}
 * request.  Data from the processing of the Command is written to the
 * supplied output stream.
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */
public class UniversalQueryRequestHandler extends RequestHandler {

    private static boolean LF = true;
    private static boolean noLF = false;

    private String encoding = null;
    private String objectName = null;
    private String objectVersion = null;
    private Encoder encoder = null;
    private DbConnectionMgr connMgr = DbConnectionMgr.getDbConnectionMgr();

    /**
     * Constructor.
     * @param writer ServletOutputStream the stream to write the results of
     * the {@link Command} to.
     */
    UniversalQueryRequestHandler(ServletOutputStream writer) {
        super(writer);
    }

    /**
     * Receives the {@link Command} to be processed and controls the processing.
     * @param command Command a single query command to be processed.
     * @throws ServletException
     */
    public void process(Command command) throws ServletException {

        try {
            validate(command);
            encoder = getEncoder(writer);
            ServicerFactory sf = new ServicerFactory();
            Servicer servicer = sf.load(objectName, objectVersion);
            //Servicer servicer = new Servicer();  /* HACK HACK HACK  Need to call ServicerFactory!! */
            servicer.validateAndLoad(command);
            stream("<payload type=\"" + objectName + "\" version=\"" + objectVersion, noLF);
            stream("\" encoding=\"" + encoding + "\">", LF);
            produceData(servicer);
        } catch(ServicerFactoryException e) {
            generateExceptionXML(e.getMessage());
        } catch(RequestHandlerException e) {
            generateExceptionXML(e.getMessage());
        }
    }

    /**
     * Creates a general error message on the output stream.
     * @param message String informational data about the error which is returned
     * to the caller via the output stream.
     * @throws ServletException
     */
    private void generateExceptionXML(String message) throws ServletException {
        stream("<payload type=\"" + objectName + "\" version=\"" + objectVersion + "\"", noLF);
        stream(" encoding=\"" + encoding + "\">", LF);
        stream("<quality error=\"1\" code=\"???\" message=\"" + message + "\"/>", LF);
        stream("</payload>", LF);
    }

    private void produceData(Servicer servicer) throws ServletException {

        Connection connection = null;
        try {
            connection = connMgr.acquire();
            queryData(servicer, connection);
        } catch(DbConnectionMgrException e) {
            stream("<quality error=\"1\" code=\"???\" message=\"" + e.getMessage() + "\"/>", LF);
            stream("</payload>", LF);
        } finally {
            try {
                connMgr.release(connection);
            } catch(DbConnectionMgrException e) {}
        }
    }

    private void queryData(Servicer servicer, Connection connection) throws ServletException {
        PreparedStatement stmt = null;
        ResultSet rs = null;
        String sql = servicer.getSql();
        try {
            stmt = connection.prepareStatement(sql);
            if(servicer.param_num > 0) stmt.setString(1, servicer.cmd.get("cid"));
            rs = stmt.executeQuery();
            marshal(servicer, rs);
        } catch(SQLException e) {
            stream("<quality error=\"1\" code=\"???\" message=\"" + e.getMessage() + "\"/>", LF);
            stream("</payload>", LF);
        } finally {
            try {if(rs != null) rs.close();
            } catch(SQLException e) {}
            try {if(stmt != null) stmt.close();
            } catch(SQLException e) {}
        }
    }

    private void marshal(Servicer servicer, ResultSet resultSet) throws SQLException,
        ServletException {
        try {
            stream("<data>", noLF);
            long recordCnt = servicer.marshal(encoder, resultSet);
            byte[] md5_digest = encoder.getMD5Digest();
            StringBuffer md5_ascii = new StringBuffer("");
            for(int i = 0; i < md5_digest.length; i++) {
                int v = (int) md5_digest[i];
                if(v < 0) v = 256 + v;
                String str = Integer.toString(v, 16);
                if(str.length() == 1) md5_ascii.append("0");
                md5_ascii.append(str);
            }
            stream("</data>", LF);
            stream("<quality error=\"0\" md5=\"" + md5_ascii + "\" records=\"" + recordCnt + "\"/>",
                   LF);
            stream("</payload>", LF);
        } catch(SQLException e) {
            stream("</data>", LF);
            throw e;
        } catch(Exception e) {
            throw new ServletException(e);
        }
    }

    /**
     * Insures required keys common to all requests exist and are valid. As a
     * side effect this class will set values internal to this class.
     *
     * @param  command  An instance of the Command class.
     * @return void
     * @except RequestHandlerExcepiton thrown if the command is invalid.
     *
     */
    private void validate(Command command) throws RequestHandlerException {
        encoding = command.remove("encoding");
        if(encoding == null)
            throw new RequestHandlerException("Missing the tag 'encoding'");
        String type = command.remove("type");
        if(type == null)
            throw new RequestHandlerException("Missing the tag 'type'");
        else {
            StringTokenizer typeComponets = new StringTokenizer(type, ":");
            if(typeComponets.countTokens() != 2)
                throw new RequestHandlerException("Malformed tag 'type', value of type: "
                                                  + typeComponets);
            else {
                objectName = typeComponets.nextToken();
                objectVersion = typeComponets.nextToken();
            }
        }
    }

    /**
     * Creates and returns an Encoder of the type set in a private class variable.
     *
     * @except RequestHandlerExcepiton thrown if the command is invalid.
     *
     */
    private Encoder getEncoder(ServletOutputStream writer) throws RequestHandlerException {
        Encoder encoder = null;
        try {
            if(encoding.compareToIgnoreCase("blob") == 0)
                encoder = new BlobTypedEncoder(writer);
            else if(encoding.compareToIgnoreCase("xml") == 0)
                throw new RequestHandlerException("XML encoding is not yet supported.");
            else if(encoding.compareToIgnoreCase("csv") == 0)
                throw new RequestHandlerException("CSV encoding is not yet supported.");
            else
                throw new RequestHandlerException("Received unknown encoding type of " + encoding);
        } catch(Exception e) {
            throw new RequestHandlerException(e.getMessage());
        }
        return encoder;
    }

    private void stream(String line, boolean lineFeed) throws ServletException {
        try {
            if(lineFeed)
                writer.println(line);
            else
                writer.print(line);

        } catch(IOException e) {
            throw new ServletException(e.getMessage());
        }
    }

}
