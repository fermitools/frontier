package gov.fnal.frontier;

import apmon.ApMon;
import java.util.Vector;
import java.net.InetAddress;

/**
 * Threaded class which records events to a remote server (monalisa).  The specific
 * server and how often the events are recorded are defined in the XML file.
 * @version $Revision$
 * @author Stephen P. White <swhite@fnal.gov>
 */
public class Monitor extends Thread {

  private long   delay;
  private Vector destList = new Vector();
  private ApMon  monalisa;
  private String nodeName;

  private long hits   = 0;  // Requires synchronized access

  /**
   * Constructor
   * @param host String 'node:port password' of the monalisa node to talk to.
   * @param delayInMilliseconds String of how many milliseconds to delay between sending
   * an event.
   * @throws Exception
   */
  public Monitor(String host, String delayInMilliseconds) throws Exception {
    Long millisecs = new Long(delayInMilliseconds);
    delay = millisecs.longValue();
    destList.add(host);
    monalisa = new ApMon(destList);
    InetAddress address = InetAddress.getLocalHost();
    nodeName =  address.getHostName();
  }

  /**
   * Increase the hit count, where this count identifies a single access to the servelet.
   */
  public synchronized void increment() {
    hits += 1;
  }

  /**
   * When started executes the thread in a loop which sends an event and sleeps for time
   * specified in the constructor.
   */
  public void run() {
    boolean loop = true;
    while (!interrupted() && loop) {
      try {
        Thread.sleep(delay);
        monalisa.sendParameter("FronTier Monitoring",nodeName,"Hits",hits);
        System.out.println("Monitoring " + nodeName + " hits: " + hits);
        resetHits();
      } catch (InterruptedException e) {
        loop = false;
        System.out.println(
            "Monitor:run received an InterruptedException - terminating thread.");
      } catch (Exception e) {
        System.out.println("Monitor Run Error: " + e);
        e.printStackTrace();
      }
    }
  }

  /**
   * Synchronized method which resets the hit counter to zero.
   */
  private synchronized void resetHits() {
    hits = 0;
  }

}
