import java.net.InetAddress;
import java.util.Vector;

import lia.Monitor.monitor.*;
import lia.Monitor.monitor.MNode;
import lia.Monitor.monitor.MonModuleInfo;
import lia.Monitor.monitor.MonitoringModule;
import lia.Monitor.monitor.Result;

import org.opennms.protocols.snmp.SnmpOctetString;
import org.opennms.protocols.snmp.*;
import org.opennms.protocols.snmp.SnmpSyntax;
import org.opennms.protocols.snmp.SnmpVarBind;


public class systemCache extends snmpMon implements MonitoringModule  {
static public String ModuleName="systemCache";

    static public String [] ResTypes = { 
	"cacheSysPageFaults",
	"cacheSysNumReads",
	"cacheMemUsage",
	"cacheCpuTime",
	"cacheCpuUsage",
	"cacheMaxResSize",
	"cacheNumObjCount",
	"cacheCurrentLRUExpiration",
	"cacheCurrentUnlinkRequests",
	"cacheCurrentUnusedFDescrCnt",
	"cacheCurrentResFileDescrCnt"
    };

static String sOid = ".1.3.6.1.4.1.3495.1.3.1";
static public String OsName = "*";

public systemCache () { 
  super( sOid, ModuleName);
  info.ResTypes = ResTypes;
  info.name = ModuleName ;
}


public String[] ResTypes () { return ResTypes; }
public String getOsName() { return OsName; }


public Object   doProcess() throws Exception {
       Vector res = results();
      
       if ( res.size() == 0 )  {
         throw new Exception ( " snmp Load failed " );
       }

       Result result  = new Result ( Node.getFarmName(), Node.getClusterName(),Node.getName(), ModuleName, ResTypes );
       result.time =  System.currentTimeMillis();

       for ( int i=0; i < res.size() ; i ++ ) {
          SnmpVarBind vb =  (SnmpVarBind )  res.elementAt(i);
          SnmpSyntax ss1 = ( SnmpSyntax ) vb.getValue();
          if ( ss1 instanceof  SnmpOctetString ) {
             SnmpOctetString ocst = ( SnmpOctetString )  ss1 ;
             String value =  new String ( ocst.getString() );
             double dvalue = Double.valueOf(value).doubleValue();
             result.param[i] = dvalue;
          }
	  else if ( ss1 instanceof  SnmpInt32 ) {
             SnmpInt32 snmpi = ( SnmpInt32 )  ss1 ;
             int dvalue = snmpi.getValue();
             result.param[i] = dvalue;
          }
	  else if ( ss1 instanceof   SnmpUInt32 ) {
	       SnmpUInt32 snmpc32 = ( SnmpUInt32)  ss1 ;
	      
	       result.param[i] = new Long(snmpc32.getValue()).doubleValue();
          }
       }

       return result;
}


static public void main ( String [] args ) {
  String host = args[0] ;
  systemCache aa = new systemCache();
  String ad = null ;
  try {
    ad = InetAddress.getByName( host ).getHostAddress();
  } catch ( Exception e ) {
    System.out.println ( " Can not get ip for node " + e );
    System.exit(-1);
  }


  MonModuleInfo info = aa.init( new MNode (args[0] ,ad, null, null), null);

 try { 
  Object bb = aa.doProcess();
  Thread th = new Thread();
  Thread.sleep( 30000 ) ;

  Result  cc = (Result) aa.doProcess();
 System.out.println ( cc );
 } catch ( Exception e ) { e.printStackTrace();}
  

}
    
 
}
