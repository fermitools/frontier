package gov.fnal.frontier;

import java.io.IOException;
import java.util.ArrayList;
import javax.servlet.ServletException;
import javax.servlet.ServletOutputStream;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import com.oreilly.servlet.CacheHttpServlet;
import java.net.URLDecoder;
import java.util.Calendar;

/**
 * Top level Frontier servlet object called by Tomcat.
 * @version $Revision$
 * @author Stephen P. White <swhite@fnal.gov>
 */
public final class Frontier extends CacheHttpServlet {

    private static final long time_expire = (1000 * 60 * 60 * 24 * 7); // 7 days, in milliseconds

    private static int count_total = 0;
    private static int count_current = 0;
    private static Boolean mutex = new Boolean(true);

    private static final String frontierVersion = "1.0";
    private static final String xmlVersion = "1.0";

    /**
     * Respond to a GET request for the content produced by this
     * servlet.  This function handles call-outs to the different
     * interfaces supported by this servlet
     * @param request HttpServletRequest The servlet request we are processing
     * @param response HttpServletResponse The servlet response we are producing
     * @throws IOException if an input/output error occurs
     * @throws ServletException if a servlet error occurs
     */
    public void doGet(HttpServletRequest request,
                      HttpServletResponse response) throws IOException, ServletException {

        int local_current;
        Identifier id;
        Calendar timestamp = Calendar.getInstance();
        DbConnectionMgr connMgr = null;
        CommandParser parser = null;
        ArrayList commandList = null;
        ServletOutputStream writer = null;

        synchronized (mutex) {
            ++count_total;
            ++count_current;
            local_current = count_current;
            id = new Identifier(count_total);
        }

        try {
            String queryString = URLDecoder.decode(request.getQueryString(), "UTF-8");
            System.out.println("<frontierLog " + timestamp.getTime()
                               + " ID: " + id.getIdentifier() + ">  "
                               + " start "
                               + " threads: " + local_current
                               + " queryString: " + queryString);

            response.setContentType("text/xml");
            response.setCharacterEncoding("US-ASCII");
            // For Squid
            response.setDateHeader("Expires", timestamp.getTimeInMillis() + time_expire);

            connMgr = DbConnectionMgr.getDbConnectionMgr();
            parser = new CommandParser();
            writer = response.getOutputStream();

            writer.println("<?xml version=\"1.0\" encoding=\"US-ASCII\"?>");
            writer.println("<frontier version=\"" + frontierVersion + "\" xmlversion=\"" +
                           xmlVersion
                           + "\">");
            try {
                commandList = parser.parse(queryString);
                connMgr.initialize();
                handleRequest(id,commandList, writer);
            } catch (CommandParserException e) {
                writer.println("<transaction payloads=\"0\"/>");
                writer.println(generateBadQualityTag(id,e));
            } catch (DbConnectionMgrException e) {
                /** @todo Need to cleaup th /transaction clauses in this class*/
                writer.println("</transaction>");
                System.out.println("Unable to obtain connection: " + e.getMessage());
                writer.println(generateBadQualityTag(id,e));
            } catch (Exception e) {
                writer.println("</transaction>");
                System.out.println("Error: " + e);
                e.printStackTrace();
                writer.println(generateBadQualityTag(id,e));
            }

        } finally {
            writer.println("</frontier>");
            writer.close();
            synchronized (mutex) {
                --count_current;
                local_current = count_current;
            }

            Calendar timestamp2 = Calendar.getInstance();
            System.out.println("<frontierLog " + timestamp2.getTime()
                               + " ID: " + id.getIdentifier() + ">  "
                               + " stop "
                               + " threads: " + local_current + " elapsedTime: "
                               + (timestamp2.getTimeInMillis() - timestamp.getTimeInMillis()));
        }
    }

    /**
     * Oversees the processing of Commands by distributing  each command to the
     * appropriate handler.
     * @param Identifier object which shows who called the method.
     * @param commandList An ArrayList of Command objects which are to be handled.
     * @param writer      The output writer.println to respond to the Command on.
     * @throws ServletException if a servlet error occurs
     */
    private void handleRequest(Identifier id, ArrayList commandList, ServletOutputStream writer)
        throws Exception {
        RequestHandler handler = null;
        int numCommands = commandList.size();
        writer.println("<transaction payloads=\"" + numCommands + "\">");
        for (int i = 0;i < numCommands;i++) {
            Command command = (Command) commandList.get(i);
            if (command.isUniversalQueryCommand())
                handler = new UniversalQueryRequestHandler(id, writer);
            else if (command.isAdminCommand())
                handler = new AdministrationRequestHandler(id, writer);
            else
                throw new ServletException("Internal error, unknown type of Command.");
            try {
                handler.process(command);
            } catch (RequestHandlerException e) {
                generateBadQualityTag(id,e);
            }
        }
        writer.println("</transaction>");
    }

    /**
     * Outputs an XML tag to the writer.println indicating an error condtion
     * @param id {@link Identifier}
     * @param message Text message to place into writer.println
     * @throws ServletException if a servlet error occurs
     */
    private String generateBadQualityTag(Identifier id, Exception e) {
        recordError(id,e);
        return "<quality error=\"1\" message=\"" + e.getMessage() + "\"/>";
    }

    /**
     * Outputs the message from the quality error to the log file.
     * @param message String text contining the error message.
     * @todo Need a debug class to handle such messages.
     */
    private void recordError(Identifier id, Exception e) {
        Calendar timestamp = Calendar.getInstance();
        System.out.println("<frontierLog " + timestamp.getTime()
                           + "ID: " + id.getIdentifier() + "> error: " + e.getMessage());
        e.printStackTrace();
    }


} // class Frontier
