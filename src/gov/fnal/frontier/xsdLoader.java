package gov.fnal.frontier;

import java.io.File;
import java.io.FileInputStream;
import java.io.ByteArrayInputStream;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.DriverManager;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.input.SAXBuilder;
import java.io.FileNotFoundException;

/**
 * This application validates all files within the specificed directory according to
 * the version of the XSD the files are created with.  If all XSD files pass this application  can
 * then store them within the frontier_descriptor table of the specified schema.
 * @author Stephen P. White <swhite@fnal.gov>
 * @version $Revision$
 */

public class xsdLoader {

    private class xmlObject {
        public String type = null;
        public String version = null;
    }

    Connection conn = null;
    PreparedStatement pstmt = null;

    public xsdLoader() {}

    private byte[] readFile(FileInputStream xsdFile) throws Exception {
        byte[] blobBytes = null;
        try {
            try {
                int byteSize = xsdFile.available();
                blobBytes = new byte[byteSize];
                xsdFile.read(blobBytes);
            } catch(Exception ex) {
                System.out.println("readFile error: " + ex.getMessage());
                throw ex;
            }
        } catch(Exception ex) {
            throw new Exception("readFile - " + ex.getMessage());
        } finally {
            xsdFile.close();
        }
        return blobBytes;
    }

    public xmlObject load(FileInputStream xsdFile) throws Exception {
        xmlObject obj = null;
        try {
            SAXBuilder builder = new SAXBuilder();
            Document doc = builder.build(xsdFile);
            Element root = doc.getRootElement();
            obj = new xmlObject();
            obj.type = root.getAttributeValue("type");
            obj.version = root.getAttributeValue("version");
            if(obj.type == null) {
                throw new Exception("The XSD is missing the tag type or it is null.");
            }
            if(obj.version == null) {
                throw new Exception("The XSD is missing the tag version or it is null.");
            }
        } catch(Exception ex) {
            throw new Exception("load - " + ex.getMessage());
        } finally {
            xsdFile.close();
        }
        return obj;
    }

    private void connect() throws Exception {
        try {
            String blobInsert = "INSERT INTO frontier_descriptors";
            blobInsert += " (name,version,xsd_type,xsd_data)";
            blobInsert += " VALUES (?,?,?,?)";
            DriverManager.registerDriver(new oracle.jdbc.driver.OracleDriver());
            String oracleUrl =
                "jdbc:oracle:thin:@(DESCRIPTION=(ADDRESS=(PROTOCOL=tcp)"
                + "(PORT=1521)(HOST=fcdflnx1.fnal.gov))(CONNECT_DATA=(SID=cdfrep01)))";
            conn = DriverManager.getConnection(oracleUrl, "frontier_prd", "frontier_prd");
            pstmt = conn.prepareStatement(blobInsert);
        } catch(Exception ex) {
            throw new Exception("connect - " + ex.getMessage());
        }

    }
    /* Inserts the blob into the blobDataTest_ table.  */
    private void blobInsert(String name, String version, String type, byte[] xsd) throws Exception {
        try {
            ByteArrayInputStream bIn = new ByteArrayInputStream(xsd, 0, xsd.length);
            pstmt.setString(1, name);
            pstmt.setString(2, version);
            pstmt.setString(3, type);
            pstmt.setBinaryStream(4, bIn, xsd.length);
            pstmt.execute();
            conn.commit();
        } catch(Exception ex) {
            throw new Exception("blobInsert - " + ex.getMessage());
        }
    }

    public int validFileList(File[] fileList) {
        int retValue = 0;
        XmlLoader loader = new XmlLoader();
        System.out.println("Validating XSD files.....");
        for(int i=0; i < fileList.length; i++) {
            try {
                loader.load(new FileInputStream(fileList[i]));
                // System.out.println(fileList[i].getName() + " -- " + "Syntax is valid.");
            } catch(LoaderException ex) {
                retValue += 1;
                System.out.println(fileList[i].getName() + " -- " + ex.getMessage());
            } catch(FileNotFoundException ex) {
                retValue += 1;
                System.out.println(fileList[i].getName() + " -- " + ex.getMessage());
            }
        }
        return retValue;
    }

    public static void main(String[] args) {
        if(args.length != 1) {
            System.out.println("xsdLoader <path_to_files>");
            System.exit(1);
        }
        try {
            File file = new File(args[0] + "/");
            File[] files = file.listFiles();
            xsdLoader loader = new xsdLoader();
            int badSyntaxCnt = loader.validFileList(files);
            if (badSyntaxCnt == 0) {
                loader.connect();
                int dataLength = 0;
                int i = 0;
                int gCnt = 0;
                int eCnt = 0;
                System.out.println("Storing XSD files into db.....");
                for (i = 0;i < files.length;i++) {
                    try {
                        xmlObject obj = loader.load(new FileInputStream(files[i]));
                        byte[] xsd = loader.readFile(new FileInputStream(files[i]));
                        loader.blobInsert(obj.type, obj.version, "xml", xsd);
                        gCnt += 1;
                        //System.out.println(files[i].getName() + " was stored,  xsd data length: "
                        //                   + xsd.length);
                        dataLength += xsd.length;
                    } catch (Exception ex) {
                        eCnt += 1;
                        System.out.println("****** " + files[i].getName()
                                           + "\n     error: " + ex.getMessage());
                    }
                }
                System.out.println("Total xsd data bytes: " + dataLength);
                System.out.println("Total Files Saved:    " + gCnt);
                System.out.println("Total Errors:         " + eCnt);
                System.out.println("Avg xsd data bytes:   " + (dataLength / i));
                loader.conn.close();
            } else if (badSyntaxCnt == 1)
                System.out.println("\nThere is " + badSyntaxCnt + " file with syntax errors.");
            else if (badSyntaxCnt > 1)
                System.out.println("\nThere are " + badSyntaxCnt + " files with syntax errors.");

        } catch(Exception ex) {
            System.out.println(ex.getMessage());
        }
    }
}
