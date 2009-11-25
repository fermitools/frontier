package gov.fnal.frontier;

import apmon.ApMon;
import java.util.Vector;
import java.net.InetAddress;

/**
 * Threaded class which records events to a remote server (monalisa).  The specific
 * server and how often the events are recorded are defined in the XML file.
 * @version $Revision$
 * @author Stephen P. White <swhite@fnal.gov>
 * @update Fail-counter added [SK]
 */
public class Monitor extends Thread 
 { 
  public static final int MAX_FAIL=10; // Maximum number of times the monitor loop
                                       // fails in a row before the monitor gives up.

  private long   delay;
  private Vector<String> destList = new Vector<String>();
  private ApMon  monalisa;
  private String nodeName;
  private int    fail_count;           // Monitor loop in a row fails counter

  private long hits   = 0;  // Requires synchronized access

  /**
   * Constructor
   * @param host String 'node:port password' of the monalisa node to talk to.
   * @param delayInMilliseconds String of how many milliseconds to delay between sending
   * an event.
   * @throws Exception
   */
  public Monitor(String host, String delayInMilliseconds) throws Exception 
   {
    Long millisecs = new Long(delayInMilliseconds);
    delay = millisecs.longValue();
    destList.add(host);
    monalisa = new ApMon(destList);
    InetAddress address = InetAddress.getLocalHost();
    nodeName =  address.getHostName();
    fail_count=0;
    Frontier.Log("MonAlisa monitor has been created: h="+host+", d="+delayInMilliseconds);
   }

  /**
   * Increments number of hits
   */
  protected void increment()
   {
    ++hits;
   }

  /**
   * When started executes the thread in a loop which sends an event and sleeps for time
   * specified in the constructor.
   */
  public void run() 
   {
    Frontier.Log("MonAlisa monitor has been started.");
    boolean loop = true;
    while (!interrupted() && loop) 
     {
      try 
       {
        if(fail_count>MAX_FAIL)
         {
          Frontier.Log("gov.fnal.frontier.Monitor:run() aborts due to "+fail_count+" fails in a row");
          return;
         }
        Thread.sleep(delay);
        monalisa.sendParameter("FronTier Monitoring",nodeName,"Hits",hits);
        Frontier.Log("Monitoring " + nodeName + " hits: " + hits);
        resetHits();
        fail_count=0;
       } 
      catch (InterruptedException e) 
       {
        loop = false;
        Frontier.Log("Monitor:run received an InterruptedException - terminating thread.");
       } 
      catch (Exception e) 
       {
        ++fail_count;
        Frontier.Log("Monitor Run Error: ",e);
       }
     }
   }

  /**
   * Synchronized method which resets the hit counter to zero.
   */
  private void resetHits() 
   {
    hits = 0;
   }
 }
