package gov.fnal.frontier;

import java.io.IOException;
import java.sql.Connection;
import java.util.StringTokenizer;
import java.util.Calendar;
import java.util.ListIterator;
import javax.servlet.ServletException;
import javax.servlet.ServletOutputStream;

import gov.fnal.frontier.codec.BlobTypedEncoder;
import gov.fnal.frontier.codec.Encoder;

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
    private boolean isMetaQuery=false;

    private String encoding = null;
    private String objectName = null;
    private String objectVersion = null;
    private DbConnectionMgr connMgr = DbConnectionMgr.getDbConnectionMgr();

    /**
     * Constructor.
     * @param id {@link Identifier}
     * @param writer ServletOutputStream the stream to write the results of
     * the {@link Command} to.
     */
    UniversalQueryRequestHandler(Identifier id, ServletOutputStream writer) {
        super(id, writer);
    }

    /**
     * Receives the {@link Command} to be processed and controls the processing.
     * All data produced is output to the stream given in the constructor.
     * @param command Command a single query command to be processed.
     * @exception ServletException
     * @return void
     */
    public void process(Command command) throws ServletException {
        Connection connection = null;
        try {
            validate(command);
            Encoder encoder = getEncoder(writer);
            /** @todo ServicerFactory needs to be a Singleton class at some point */
            ServicerFactory sf = new ServicerFactory();
            Servicer servicer = sf.load(objectName, objectVersion);
            servicer.validateAndLoad(command);
	    
	    if(command.isMetaQueryCommand())
	     {
              stream("<payload meta=\"" + objectName + "\" version=\""+objectVersion+"\" encoding=\""+encoding + "\">", LF);
	      produceMetaData(servicer,encoder);
	     }
	    else
	     {
              stream("<payload type=\"" + objectName + "\" version=\""+objectVersion+"\" encoding=\""+encoding + "\">", LF);
              connection = connMgr.acquire();
              produceData(servicer, connection, encoder);
	     }
        } catch(ServicerFactoryException e) {
            generateExceptionXML(e);
        } catch(RequestHandlerException e) {
            generateExceptionXML(e);
        } catch(DbConnectionMgrException e) {
            generateExceptionXML(e);
        } finally {
            try {
                connMgr.release(connection);
            } catch(DbConnectionMgrException e) {
                recordError(e);
            }
        }
    }
    /**
     * Internal method which oversees calling the {@link Servicer} and wrapping the call with
     * the correct XML.
     * @param servicer Servicer the instance to call for obtaining database data.
     * @param connection Connection an active database connection.
     * @param encoder Encoder the class responsible for marshelling the Servicer's data.
     * @throws ServletException
     */
    private void produceData(Servicer servicer, Connection connection, Encoder encoder) throws
        ServletException {
        try {
            stream("<data>", noLF);
            long recordCnt = servicer.process(connection, encoder);
            stream("</data>", LF);
            stream("<quality error=\"0\" md5=\""
                   + md5Digest(encoder)
                   + "\" records=\""
                   + recordCnt + "\"/>", LF);
            stream("</payload>", LF);
        } catch(ServicerException e) {
            recordError(e);
            stream("</data>", LF);
            stream("<quality error=\"1\" message=\"" + e.getMessage() + "\"/>", LF);
            stream("</payload>", LF);
        } catch(ServletException e) {
            recordError(e);
            stream("</data>", LF);
            stream("<quality error=\"1\" message=\"" + e.getMessage() + "\"/>", LF);
            stream("</payload>", LF);
        }
    }

    
    private void produceMetaData(Servicer servicer, Encoder encoder) throws ServletException 
     {
      try 
       {
        stream("<data>", noLF);
	for(ListIterator lit=servicer.getAttributes();lit.hasNext();)
	 {
	  Attribute attr=(Attribute)lit.next();
	  encoder.writeString(attr.getField());
	  encoder.writeString(attr.getType());
	 }
	encoder.writeEOR();
	encoder.flush();
        stream("</data>", LF);
        stream("<quality error=\"0\" md5=\""+ md5Digest(encoder)+ "\" records=\"1\"/>", LF);
        stream("</payload>", LF);
        } catch(Exception e) {
            recordError(e);
            stream("</data>", LF);
            stream("<quality error=\"1\" message=\"" + e.getMessage() + "\"/>", LF);
            stream("</payload>", LF);
        }
    }
    
    
    /**
     * Produces md5 digest.
     * @param encoder Encoder
     * @throws ServletException
     * @return StringBuffer
     */
    private StringBuffer md5Digest(Encoder encoder) throws ServletException {
        StringBuffer md5_ascii = null;
        try {
            byte[] md5_digest = encoder.getMD5Digest();
            md5_ascii = new StringBuffer("");
            for(int i = 0; i < md5_digest.length; i++) {
                int v = (int) md5_digest[i];
                if(v < 0) v = 256 + v;
                String str = Integer.toString(v, 16);
                if(str.length() == 1) md5_ascii.append("0");
                md5_ascii.append(str);
            }
        } catch(Exception e) {
            throw new ServletException(e);
        }
        return md5_ascii;
    }

    /**
     * Insures required keys common to all requests exist and are valid. As a
     * side effect this class will set values internal to this class.
     * @param  command  An instance of the Command class.
     * @exception RequestHandlerExcepiton thrown if the command is invalid.
     * @return void
     */
    private void validate(Command command) throws RequestHandlerException {
        encoding = command.remove("encoding");
        if(encoding == null)
            throw new RequestHandlerException("Missing the tag 'encoding'");
        String type = command.remove("type");
        if(type == null) type = command.remove("meta");
	if(type == null)
            throw new RequestHandlerException("Missing one of the tag 'type' or 'meta'");
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
     * Creates and returns an sub-class of Encoder and returns it to the caller.  The
     * sub-class created is based on a private variable which was filled from the URL.
     * @exception RequestHandlerExcepiton thrown if the command is invalid.
     * @return Encoder
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

    /**
     * Creates a general error message on the output stream.
     * @param exception Exception instance.
     * to the caller via the output stream.
     * @exception ServletException
     */
    private void generateExceptionXML(Exception e) throws ServletException {
        recordError(e);
        stream("<payload type=\"" + objectName + "\" version=\"" + objectVersion + "\"", noLF);
        stream(" encoding=\"" + encoding + "\">", LF);
        // Replace any double quote with a single one.  When outputting the data to the stream.
        stream("<quality error=\"1\" message=\"" + e.getMessage().replace('"','\'') + "\"/>", LF);
        stream("</payload>", LF);
    }

    /**
     * Outputs the message from the quality error to the log file.
     * @param message String text contining the error message.
     * @todo Need a debug class to handle such messages.
     */
    private void recordError(Exception e) {
        Calendar timestamp = Calendar.getInstance();
        System.out.println("<frontierCMSLog " + timestamp.getTime()
                           + " ID: " + id.getIdentifier() + "> error: " + e.getMessage());
        e.printStackTrace();
    }

    /**
     * Outputs data to the Stream.  This is a utility method which simply allows the exception
     * to be caught in one place.
     * @param line String data to be output to the stream.
     * @param lineFeed boolean true if a newline should be placed at the end of the data.
     * @exception ServletException
     */
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
