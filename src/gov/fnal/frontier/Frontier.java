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
        int id;
        Calendar timestamp = Calendar.getInstance();
        DbConnectionMgr connMgr = null;
        CommandParser parser = null;
        ArrayList commandList = null;
        ServletOutputStream writer = null;

        synchronized (mutex) {
            id = count_total;
            ++count_total;
            ++count_current;
            local_current = count_current;
        }

        String queryString = URLDecoder.decode(request.getQueryString(),"UTF-8");
        System.out.println("frontierLog " + timestamp.getTime() + " start " + id + " "
                           + local_current + " " + queryString);

        response.setContentType("text/xml");
        response.setCharacterEncoding("US-ASCII");
        // For Squid
        response.setDateHeader("Expires", timestamp.getTimeInMillis() + time_expire);

        connMgr = DbConnectionMgr.getDbConnectionMgr();
        parser = new CommandParser();
        writer = response.getOutputStream();

        writer.println("<?xml version=\"1.0\" encoding=\"US-ASCII\"?>");
        writer.println("<frontier version=\"" + frontierVersion + "\" xmlversion=\"" + xmlVersion
                       + "\">");
        try {
            commandList = parser.parse(queryString);
            connMgr.initialize();
            handleRequest(commandList, writer);
        } catch (CommandParserException e) {
            writer.println("<transaction payloads=\"0\"/>");
            writer.println(generateBadQualityTag(e.getMessage()));
        } catch (DbConnectionMgrException e) {
            /** @todo Need to cleaup th /transaction clauses */
            writer.println("</transaction>");
            System.out.println("Unable to obtain connection: " + e.getMessage());
            writer.println(generateBadQualityTag(e.getMessage()));
        } catch (Exception e) {
            writer.println("</transaction>");
            System.out.println("Error: " + e);
            e.printStackTrace();
            writer.println(generateBadQualityTag(e.getMessage()));
        }
        writer.println("</frontier>");

        writer.close();

        synchronized (mutex) {
            --count_current;
            local_current = count_current;
        }

        Calendar timestamp2 = Calendar.getInstance();
        System.out.println("frontierLog " + timestamp2.getTime() + " stop  " + id + " "
                           + local_current + " "
                           + (timestamp2.getTimeInMillis() - timestamp.getTimeInMillis()));
    }

    /**
     * Oversees the processing of Commands by distributing  each command to the
     * appropriate handler.
     * @param commandList An ArrayList of Command objects which are to be handled.
     * @param writer      The output writer.println to respond to the Command on.
     * @throws ServletException if a servlet error occurs
     */
    private void handleRequest(ArrayList commandList, ServletOutputStream writer) throws Exception {
        RequestHandler handler = null;
        int numCommands = commandList.size();
        writer.println("<transaction payloads=\"" + numCommands + "\">");
        for (int i = 0;i < numCommands;i++) {
            Command command = (Command) commandList.get(i);
            if (command.isUniversalQueryCommand())
                handler = new UniversalQueryRequestHandler(writer);
            else if (command.isAdminCommand())
                handler = new AdministrationRequestHandler(writer);
            else
                throw new ServletException("Internal error, unknown type of Command.");
            try {
                handler.process(command);
            } catch (RequestHandlerException e) {
                generateBadQualityTag(e.getMessage());
            }
        }
        writer.println("</transaction>");
    }

    /**
     * Outputs an XML tag to the writer.println indicating an error condtion
     * @param message Text message to place into writer.println
     * @throws ServletException if a servlet error occurs
     */
    private String generateBadQualityTag(String message) {
        return "<quality error=\"1\" code=\"???\" message=\"" + message + "\"/>";
    }

} // class Frontier
