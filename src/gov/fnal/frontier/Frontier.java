package gov.fnal.frontier;

import java.io.IOException;
import javax.servlet.ServletException;
import javax.servlet.ServletOutputStream;
import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;
import javax.servlet.ServletContext;
import javax.servlet.ServletConfig;
import com.oreilly.servlet.CacheHttpServlet;
import java.net.URLDecoder;
import java.util.*;
import java.text.SimpleDateFormat;
import java.io.*;
import apmon.*;

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
    private static final SimpleDateFormat date_fmt=new SimpleDateFormat("MM/dd/yy HH:mm:ss.SSS z Z");

    private static String conf_server_name="not_set";
    private static String conf_ds_name;
    private static String conf_xsd_table;
    private static String confMonitorNode;
    private static String confMonitorMillisDelay;


    public static String getDsName(){return conf_ds_name;}
    public static String getXsdTableName(){return conf_xsd_table;}

    private static Monitor monitor;

    /**
     * Overrides the convenience method of GenericServlet and is called by it during
     * object construction.
     * @param sc ServletConfig
     */
    public void init(ServletConfig sc){
      System.out.println("FroNtier initilization started...");
      try
       {
         conf_server_name=sc.getInitParameter("ServerName");
         conf_ds_name=sc.getInitParameter("DataSourceName");
         conf_xsd_table=sc.getInitParameter("XsdTableName");
         confMonitorNode=sc.getInitParameter("MonitorNode");
         confMonitorMillisDelay=sc.getInitParameter("MonitorMillisDelay");

        if(conf_server_name==null) throw new Exception("ServerName is missing in FrontierConfig");
        if(conf_ds_name==null) throw new Exception("DataSourceName is missing in FrontierConfig");
        if(conf_xsd_table==null) throw new Exception("XsdTableName is missing in FrontierConfig");

        if((confMonitorNode!=null) && (confMonitorMillisDelay!=null)) {
          monitor = new Monitor(confMonitorNode, confMonitorMillisDelay);
          monitor.setDaemon(true);
          monitor.start();
          System.out.println("Monitor Started, will record events every " +
                             confMonitorMillisDelay + " milliseconds.");
        } else
          System.out.println("Monitor was NOT started.");
       }
      catch(Exception e)
       {
         System.out.println("FRONTIER ERROR: "+e);
         e.printStackTrace();
       }
       System.out.println("FroNtier initilization completed.");
     }


    public static void Log(String msg)
     {
      StringBuffer buf=new StringBuffer("");
      buf.append(conf_server_name);
      buf.append(' ');
      buf.append(date_fmt.format(new java.util.Date()));
      buf.append(' ');
      buf.append(Thread.currentThread().getName());
      buf.append(' ');
      buf.append(msg);
      System.out.println(buf.toString());
     }


    public static void Log(String msg,Exception e)
     {
      ByteArrayOutputStream baos=new ByteArrayOutputStream();
      PrintWriter pw=new PrintWriter(baos);
      e.printStackTrace(pw);
      pw.flush();
      Log(msg+"\n"+baos.toString());
     }

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
                      HttpServletResponse response) throws IOException, ServletException
     {
        int local_id;
        //Identifier id;
        long timestamp = (new java.util.Date()).getTime();
        DbConnectionMgr connMgr = null;
        CommandParser parser = null;
        ArrayList commandList = null;
        ServletOutputStream writer = null;
        String xFrontierId = null;

        synchronized (mutex) {
            ++count_total;
            ++count_current;
            local_id=count_total;
            //id = new Identifier(count_total);
        }

        Thread.currentThread().setName("id="+local_id);

        try {
            String queryString = URLDecoder.decode(request.getQueryString(), "UTF-8");
            StringBuffer client_desc=new StringBuffer("");
            client_desc.append("start threads:");
            client_desc.append(count_current);
            client_desc.append(" query ");
            client_desc.append(queryString);
            client_desc.append(" raddr ");
            client_desc.append(request.getRemoteAddr());
            client_desc.append(" frontier-id: ");
            xFrontierId = request.getHeader("x-frontier-id");
            if (xFrontierId==null) xFrontierId = "Unknown";
            client_desc.append(xFrontierId);
            for(Enumeration en=request.getHeaderNames();en.hasMoreElements();)
             {
              String name=(String)en.nextElement();
              if(name.compareToIgnoreCase("via")==0 || name.compareToIgnoreCase("x-forwarded-for")==0)
               {
                client_desc.append(' ');
                client_desc.append(name);
                client_desc.append(": ");
                client_desc.append(request.getHeader(name));
               }
             }
            Log(client_desc.toString());

            response.setContentType("text/xml");
            response.setCharacterEncoding("US-ASCII");
            // For Squid
            response.setDateHeader("Expires",timestamp+time_expire);

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
                handleRequest(commandList, writer);
            } catch (CommandParserException e) {
                writer.println("<transaction payloads=\"0\"/>");
                writer.println(generateBadQualityTag(e));
            } catch (DbConnectionMgrException e) {
                /** @todo Need to cleaup th /transaction clauses in this class*/
                writer.println("</transaction>");
                Log("Unable to obtain connection: " + e.getMessage());
                writer.println(generateBadQualityTag(e));
            } catch (Exception e) {
                writer.println("</transaction>");
                Log("Error: " + e,e);
                writer.println(generateBadQualityTag(e));
            }

        } finally {
            writer.println("</frontier>");
            writer.close();
            monitor.increment();
            synchronized (mutex) {
                --count_current;
            }

         Long elapsedTime = new Long((new java.util.Date()).getTime()-timestamp);
         Log("stop threads:"+count_current+" ET="+(elapsedTime.longValue()));
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
    private void handleRequest(ArrayList commandList, ServletOutputStream writer)
        throws Exception {
        RequestHandler handler = null;
        int numCommands = commandList.size();
        writer.println("<transaction payloads=\"" + numCommands + "\">");
        for (int i = 0;i < numCommands;i++) {
            Command command = (Command) commandList.get(i);
            if (command.isUniversalQueryCommand() || command.isMetaQueryCommand())
                handler = new UniversalQueryRequestHandler(writer);
            else if (command.isAdminCommand())
                handler = new AdministrationRequestHandler(writer);
            else
                throw new ServletException("Internal error, unknown type of Command.");
            try {
                handler.process(command);
            } catch (RequestHandlerException e) {
                generateBadQualityTag(e);
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
    private String generateBadQualityTag(Exception e)
     {
      Log("ERROR: "+e,e);
      return "<quality error=\"1\" message=\"" + e.getMessage() + "\"/>";
     }


} // class Frontier
