/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */

package  com.pingtel.pds.pgs.sipxchange.replication;

import java.io.BufferedReader;
import java.io.BufferedWriter;
import java.io.DataOutputStream;
import java.io.FileInputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.ArrayList;
import java.util.Iterator;

import org.apache.log4j.Logger;

import com.pingtel.pds.common.PathLocatorUtil;
import com.pingtel.pds.common.https.HttpsUtil;
import com.pingtel.pds.pgs.sipxchange.SatelliteComponent;
import com.pingtel.pds.pgs.sipxchange.SatelliteLocation;
import com.pingtel.pds.pgs.sipxchange.SatelliteManager;

/**
 *
 *  ReplicationManager .
 *  manages the task of replicating data to satellite components.
 *
 * @author Pradeep Paudyal
 */
public class ReplicationManager
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////



//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private static ReplicationManager m_manager = new ReplicationManager();

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    private ReplicationManager () {
        HttpsUtil.init();
    }



//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    public static ReplicationManager getInstance(){
        return m_manager;
    }

    /**
     * sends payload data to this particular component.
     */
    public void replicateData( byte[] payload ,
                                SatelliteLocation  location,
                                SatelliteComponent component,
                                String payLoadType,
                                String targetDataName  ) throws  ReplicationException{

        ArrayList arrayListErrors = new ArrayList();
        String xmlData = generateXMLDataToPost(  payload,
                                                 component.getType(),
                                                 payLoadType,
                                                 targetDataName,
                                                 location );

        //if debug is turned on, write the data you are going to post to a file.
        Logger cat = Logger.getLogger( "pgs" );
        if( cat.isDebugEnabled() ){
            try{
                PrintWriter out
                    = new PrintWriter(new BufferedWriter(new FileWriter
                                ("xmlData"+location.getLocationID()+".xml")));
                out.println(xmlData);
                out.flush();
                out.close();
            }catch( Exception e ){
                e.printStackTrace();
            }
        }

        ReplicationError error = postData(xmlData, location );
        if( error != null ){
            arrayListErrors.add( error );
        }
        if( arrayListErrors.size() > 0 )
            throw new ReplicationException(arrayListErrors);

    }

    /**
     * sends payload data to all components of this type.
     */
    public void replicateData( byte[] payload ,
                                    int componentType,
                                    String payLoadType,
                                    String targetDataName  ) throws  ReplicationException{

         //needs to generate a xml file and do a http post
         //to the URL.
         ArrayList arrayListErrors = new ArrayList();
         Iterator iterator = SatelliteManager.getInstance().getAllLocations().iterator();
         while( iterator.hasNext() ) {
            SatelliteLocation location =
                            (SatelliteLocation)iterator.next();
            if( location.hasComponents(componentType) ){
                long d1 = new java.util.Date().getTime();
                String xmlData = generateXMLDataToPost(  payload,  componentType,
                                        payLoadType, targetDataName,  location );
                long d2 = new java.util.Date().getTime();
                System.out.println ( "*it took " + (d2-d1) + " to do generateXMLDataToPost(  ");

                //if debug is turned on, write the data you are going to post to a file.
                 Logger cat = Logger.getLogger( "pgs" );
                 if( cat.isDebugEnabled() ){
                    try{
                        long d3 = new java.util.Date().getTime();
                        String path =
                            PathLocatorUtil.getInstance().getPath( PathLocatorUtil.PGS_LOGS_FOLDER, PathLocatorUtil.PGS );
                        PrintWriter out= new PrintWriter(new BufferedWriter
                                (new FileWriter(path + "encodedPostData_"+targetDataName+".xml")));
                        out.println(xmlData);
                        out.flush();
                        out.close();
                        long d4 = new java.util.Date().getTime();
                        System.out.println ( "*it took " + (d4-d3) + " to do write file to disk");
                    }catch( Exception e ){
                        e.printStackTrace();
                    }
                }
                long d5 = new java.util.Date().getTime();
                ReplicationError error = postData(xmlData, location ) ;
                long d6 = new java.util.Date().getTime();
                System.out.println ( "*it took " + (d6-d5) + " to postData(xmlData, location )");

                if( error != null ){
                    arrayListErrors.add( error );
                }
            }
        }
        if( arrayListErrors.size() > 0 )
            throw new ReplicationException(arrayListErrors);
    }




    /**
     * Test method.
     */
    public static void main( String args[] ){
        if( ( args == null ) || (args.length <= 3) ){
            System.out.println(" Usage: java com.pingtel.pds.pgs.sipxchange.replication.ReplicationManager "+
                " <payload file>  <payload type(file, database)>  <target componenttype( 1(media), 2(comm)>  <payload targetname>");
            return;
        }
        String strPayloadFile          = args[0];
        String strPayloadType          = args[1];
        String strTargetComponentType  = args[2];
        String strPayloadTargetName = null;
        if( args.length > 3 )
            strPayloadTargetName    = args[3];
        int iComponentType = -1;
        if( strTargetComponentType.equals("2") )
            iComponentType = 2;
        else if( strTargetComponentType.equals("1") )
            iComponentType = 1;

         ReplicationManager replicationManager = ReplicationManager.getInstance();
         System.out.println("***********************************************");
         Iterator locIterator = SatelliteManager.getInstance().getAllLocations().iterator();
         while( locIterator.hasNext() ) {
            SatelliteLocation location =
                            (SatelliteLocation)locIterator.next();
            Iterator comIterator = location.getComponents(iComponentType).iterator();
            while( comIterator.hasNext() ) {
                 SatelliteComponent component =
                            (SatelliteComponent)comIterator.next();
                System.out.println("target url is " + location.getReplicationURL());
                System.out.println("target type is " + component.getType());
                System.out.println("target location is " + location.getLocationID());
                System.out.println("target id is " + component.getComponentID());
                System.out.println("----------------------------------------------");
            }
        }

         byte[] bytes = null;
         try{
            FileInputStream in = new FileInputStream(strPayloadFile);
            bytes = new byte[in.available()];
            in.read(bytes);
         }catch( Exception e ){
            e.printStackTrace();
         }
         if( bytes != null ){
            try{
                ReplicationManager.getInstance().replicateData( bytes, iComponentType,
                              strPayloadType, strPayloadTargetName );
            }catch( ReplicationException e ){
                Iterator errors =   e.getReplicationErrors().iterator();
                while( errors.hasNext() ){
                  ReplicationError error = (ReplicationError)errors.next() ;
                  if( error != null )
                    System.out.println( "Error : "+ error);
                }
            }
         }
    }
//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////



    /**
     * posts the xmlData to the URL given.
     * To DO: Should add error handling here.
     */
    private  ReplicationError  postData( String xmlData , SatelliteLocation location){
        ReplicationError error = null;

        try{
            long d1 = new java.util.Date().getTime();
            String param = xmlData;
            if (location.getReplicationURL() == null)
            {
                System.out.println ( "no replication url specified, skipping replication");
            } 
            else
            {
                URL replicateURL = new URL(location.getReplicationURL());
                HttpURLConnection urlConn = (HttpURLConnection)replicateURL.openConnection();
                urlConn.setDoOutput(true);
                urlConn.setRequestMethod("POST");
                urlConn.setRequestProperty("Content-length",String.valueOf(param.length()));
                urlConn.setRequestProperty("Content-Type","application/x-www-form-urlencoded");
                urlConn.connect();
                DataOutputStream postWriter=new DataOutputStream(urlConn.getOutputStream());
                postWriter.writeBytes(param);
                postWriter.flush();
                postWriter.close();
    
                long d2 = new java.util.Date().getTime();
                System.out.println ( "*it took " + (d2-d1) + " to send data over URL");
                //System.out.println("finished posting data " );
                long d3 = new java.util.Date().getTime();
                String strResponseMessage = urlConn.getResponseMessage();
                long d4 = new java.util.Date().getTime();
                System.out.println ( "*it took " + (d4-d3) + " to send data over URL");
    
                long d5 = new java.util.Date().getTime();
                //System.out.println(" response message is " + strResponseMessage );
                //System.out.println(" error response if any is  " + urlConn.getHeaderField("ErrorInReplication") );
                String strResponseBody = "";
    
                BufferedReader reader = new BufferedReader(new InputStreamReader(urlConn.getInputStream()));
                String line = null;
                while( (line = reader.readLine()) != null ){
                      strResponseBody += line;
                }
                if( ! strResponseBody.startsWith("replication was successful") ){
                    //String  strHeaderReplicationError = urlConn.getHeaderField("ErrorInReplication");
                    //if( strHeaderReplicationError != null ){
                    error = new ReplicationError( ReplicationError.COMPONENT_ERROR,
                                                                   strResponseBody,
                                                                   location, null );
                    //}
                }
    
                long d6 = new java.util.Date().getTime();
                System.out.println ( "*it took " + (d6-d5) + " to process response from URL");
                System.out.println(" response body is: " + strResponseBody );
            }


        }
        catch( IOException e ) {
            Logger log = Logger.getLogger( "pgs" );
            log.fatal("ReplicationError.COMMUNICATION_ERROR", e);
            error = new ReplicationError( ReplicationError.COMMUNICATION_ERROR,
                                                           getStackTraceAsString(e),
                                                           location, null );
        }
        return error;
    }

    /**
     * generates xml data to post to a URL.
     */
    private String generateXMLDataToPost( byte[] payload, int componentType,
                        String dataType, String targetDataName, SatelliteLocation location ){

        Iterator  componentIterator =
           location.getComponents( componentType ).iterator();

        StringBuffer buffer = new StringBuffer();
        buffer.append("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?> ");

        buffer.append("\n <!-- This file proposes a XML format (envelope, really) for sending  ");
        buffer.append("\n between the configuration server and replication target components.  Here, ");
        buffer.append("\n the XML allows you to send one or more different bundles. -->  ");
        /**
         * TiXml does not like DTDS, so removing it.
         */
         /*
        buffer.append("\n <!DOCTYPE replicationdata [ ");
        buffer.append("\n     <!ELEMENT replicationdata (data+) >");
        buffer.append("\n");
        buffer.append("\n     <!-- since payload element is of type #CDATA, the value will not be");
        buffer.append("\n     parsed by the xml parser.  So, that value can be anything,");
        buffer.append("\n     from xml data to any other format of data( like user-config type data) -->");
        buffer.append("\n     <!ELEMENT data ( payload )>");
        buffer.append("\n");
        buffer.append("\n     <!ATTLIST data type       ( database | file   ) #REQUIRED>");
        buffer.append("\n     <!ATTLIST data action     ( replace  | update ) #REQUIRED>");
        buffer.append("\n     <!ATTLIST data target_data_name CDATA #REQUIRED>");
        buffer.append("\n");
        buffer.append("\n     <!-- if both component_id and component_type are not present,");
        buffer.append("\n          send it to all the components residing in the host -->");
        buffer.append("\n");
        buffer.append("\n     <!--if component id is present, send this data to that");
        buffer.append("\n                                  particular component only -->");
        buffer.append("\n     <!ATTLIST data target_component_id CDATA #IMPLIED>");
        buffer.append("\n");
        buffer.append("\n     <!--if component type is present but component_id is not present,");
        buffer.append("\n           send this data to all the components of this type -->");
        buffer.append("\n     <!ATTLIST data target_component_type (media-server | config-server | comm-server)  #IMPLIED>");
        buffer.append("\n");
        buffer.append("\n     <!ELEMENT payload (#PCDATA) >");
        buffer.append("\n]>");
        buffer.append("\n");
        */
        buffer.append("\n<replicationdata>");

        try{
            String path =
                PathLocatorUtil.getInstance().getPath( PathLocatorUtil.PGS_LOGS_FOLDER, PathLocatorUtil.PGS );
            PrintWriter out= new PrintWriter(new BufferedWriter
                    (new FileWriter(path + "unencodedPayload_"+targetDataName+".xml")));

            out.println(new String(payload,"UTF8"));
            out.flush();
            out.close();
        }catch( Exception e ){
            e.printStackTrace();
        }

        String strPayload = Base64.encodeBytes(payload);
        while( componentIterator.hasNext() ){
             SatelliteComponent component =
                                        (SatelliteComponent)componentIterator.next();

            buffer.append("\n    <!-- text clob data example -->");
            buffer.append("\n    <data type=\""+dataType+"\" action=\"replace\" target_data_name=\""+targetDataName+"\" "+
                          "    target_component_type=\""+component.getTypeAsString()+"\" target_component_id=\""+component.getComponentID() +"\"  >");
            buffer.append("\n");
            buffer.append("\n       <payload>");

            buffer.append( strPayload  );
            buffer.append("</payload> ");
            buffer.append("\n    </data>");
        }
        buffer.append("\n</replicationdata>");
        return buffer.toString();

    }



    /**
     * Converts a stack trace to a String.
     */
    private String getStackTraceAsString( Throwable e )
    {
        StringWriter sw = new StringWriter();
        PrintWriter out = new PrintWriter(sw);
        if( e != null )
        {
            e.printStackTrace(out);
        }
        return sw.getBuffer().toString();
    }

//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////


//////////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////


}
