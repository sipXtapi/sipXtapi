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

package  com.pingtel.pds.pgs.sipxchange.process;

import com.pingtel.pds.pgs.sipxchange.SatelliteComponent;
import com.pingtel.pds.pgs.sipxchange.SatelliteLocation;
import com.pingtel.pds.pgs.sipxchange.SatelliteManager;
import com.pingtel.pds.common.https.HttpsUtil;
import com.pingtel.pds.common.XMLSupport;
import org.jdom.Document;
import org.jdom.Element;
import org.jdom.JDOMException;
import org.jdom.input.DOMBuilder;

import java.io.IOException;
import java.io.InputStream;
import java.io.PrintStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.util.*;


/**
 *  ProcessManager
 *
 *
 * @author Pradeep Paudyal
 */
public class ProcessManager
{
//////////////////////////////////////////////////////////////////////////////
// Constants
////
    public final String CONFIG_SERVER_GROUP = "ConfigServer";
    public final String MEDIA_SERVER_GROUP = "MediaServer";
    public final String COMM_SERVER_GROUP = "CommServer";
    public final String MISC_GROUP = "Misc";

//////////////////////////////////////////////////////////////////////////////
// Attributes
////
    private static ProcessManager m_manager = new ProcessManager();
    private static String PROCESS_STATUS_URL = "?command=status";

//////////////////////////////////////////////////////////////////////////////
// Construction
////
    private ProcessManager(){
        HttpsUtil.init();
    }


//////////////////////////////////////////////////////////////////////////////
// Public Methods
////

    public static ProcessManager getInstance(){
        return m_manager;
    }

    /**
     * Hashtable of Collections
     */
    public  Hashtable  getAllProcessGroups(){
         Hashtable hashLocationGroups = new Hashtable();
         Iterator iterator = SatelliteManager.getInstance().getAllLocations().iterator();
         while( iterator.hasNext() ) {
            SatelliteLocation location =
                            (SatelliteLocation)iterator.next();
            hashLocationGroups.put(location, getProcessGroups(location) );
         }
         return hashLocationGroups;
    }


    public Collection getAllProcesses(SatelliteLocation location){
       return getAllProcesses( location, null);
    }

    public Collection getAllProcesses(SatelliteLocation location, Document doc){
        ArrayList retCol = new ArrayList();
        Collection colProcessGroups = null;
        if( doc != null ){
          colProcessGroups = getProcessGroups( doc);
        }else{
          colProcessGroups =getProcessGroups( location);
        }
         if( colProcessGroups != null ){
            for( Iterator groupIterator=colProcessGroups.iterator(); groupIterator.hasNext();){
                ProcessGroup group = (ProcessGroup)groupIterator.next();
                Collection colProcesses = group.getProcesses();
                if( colProcesses != null ){
                    for( Iterator processIterator=colProcesses.iterator(); processIterator.hasNext();){
                        retCol.add( (ProcessDefinition)processIterator.next());
                    }
                }
            }
         }
         return retCol;
    }

    public InputStream getProcessInfoXMLData( SatelliteLocation location ){
        InputStream inputStream = null;

        try{
            URL agentURL = new URL(location.getAgentURL()+PROCESS_STATUS_URL );
            HttpURLConnection urlConn = (HttpURLConnection)agentURL.openConnection();
            urlConn.setDoOutput(true);
            urlConn.setRequestMethod("GET");
            urlConn.connect();
            inputStream = urlConn.getInputStream();
         }catch( IOException e ){
            //needs to be more specific.
            e.printStackTrace();
        }
        return inputStream;
    }

     public Document getProcessInfoDocument( SatelliteLocation location ){
        Document doc = null;

        try{
            URL agentURL = new URL(location.getAgentURL()+PROCESS_STATUS_URL );
            HttpURLConnection urlConn = (HttpURLConnection)agentURL.openConnection();
            urlConn.setDoOutput(true);
            urlConn.setRequestMethod("GET");
            urlConn.connect();
            InputStream inputStream = urlConn.getInputStream();

            if( inputStream != null ){
                DOMBuilder domBuilder = new DOMBuilder();
                org.jdom.adapters.XercesDOMAdapter cdoma = 
                    new org.jdom.adapters.XercesDOMAdapter();
                doc = domBuilder.build( cdoma.getDocument(inputStream, false ) );
            }
        }catch( IOException e ){
            //needs to be more specific.
            e.printStackTrace();
        }catch ( JDOMException ex ) {
                ex.printStackTrace();
        }
        return doc;
    }

    public Element getProcessDefinitionsData( SatelliteLocation location,
                                                String strProcessID,
                                                 Document doc ){


         Element retElement = null;

        try{
            if( doc != null ){
                // XCF-154 - warning, jdom will throw error if property is added more than
                // once to some DOM.  If this happens, must call Element.detach() first.
                Element processRootElement = (Element)(doc.getRootElement().clone());
                Collection groups = processRootElement.getChildren( "group" );
                for(  Iterator iGroups = groups.iterator(); iGroups.hasNext(); ) {
                    Element elGroup      = (Element) iGroups.next();
                    Collection colProcessesElements = elGroup.getChildren("process");
                    for( Iterator iProcesses = colProcessesElements.iterator(); iProcesses.hasNext(); ){
                        Element elProcess = (Element)iProcesses.next();
                        String  strName    = elProcess.getAttribute( "name" ).getValue();
                        if( strName.equals(strProcessID) ){
                            retElement = (Element)(elProcess.clone());
                            break;
                        }
                    }
                    if( retElement != null ){
                        break;
                    }
                }
            }
        }catch ( Exception ex ) {
                ex.printStackTrace();
        }
        return retElement;

    }

  public Element getProcessDefinitionsData( SatelliteLocation location,
                                                String strProcessID){
    Document doc = getProcessInfoDocument( location );
    return getProcessDefinitionsData( location, strProcessID, doc);
  }
     /**
     *
     */
    public  Collection  getProcessGroups( SatelliteLocation location){
        Collection colProcessGroups = null;

        Document doc = getProcessInfoDocument( location );
        if( doc != null ){
            colProcessGroups = getProcessGroups( doc );
        }
        return colProcessGroups;
    }

    /**
     * gets a ProcessGroup with the given groupname residing in the given location.
     */
    public  ProcessGroup  getProcessGroup(SatelliteLocation location, String strGroup){
        ProcessGroup retGroup = null;
        InputStream inputStream = getProcessInfoXMLData( location );
        if( inputStream != null ){
            Collection colProcessGroups = getProcessGroups( inputStream );
            if( colProcessGroups != null ){
                for( Iterator groupIterator=colProcessGroups.iterator(); groupIterator.hasNext();){
                    ProcessGroup group = (ProcessGroup)groupIterator.next();
                    if( group.getGroupID().equals(strGroup) ){
                        retGroup = group;
                        break;
                    }
                }
            }
        }
        return retGroup;
    }

    public ProcessDefinition getProcess(SatelliteLocation location,
                                                  String strProcessID ){
        Document doc = null;
        return getProcess( location, strProcessID, doc );
    }

    public ProcessDefinition getProcess(SatelliteLocation location,
                                                  String strProcessID,
                                                Document doc){
        ProcessDefinition retProcess = null;
        Collection colGroups = null;
        if( doc != null ){
             colGroups = ProcessManager.getInstance().getProcessGroups(doc);
        }else{
            colGroups = ProcessManager.getInstance()
                            .getProcessGroups(location);
        }
        if(  colGroups != null ){
            for( Iterator iGroups = colGroups.iterator(); iGroups.hasNext(); ){
                ProcessGroup group = (ProcessGroup)iGroups.next();
                Collection colProcesses = group.getProcesses();
                for( Iterator iProcesses = colProcesses.iterator(); iProcesses.hasNext(); ){
                    ProcessDefinition process = (ProcessDefinition)iProcesses.next();
                    if( process.getName().equals(strProcessID) ){
                        retProcess = process;
                        break;
                    }
                }
                if( retProcess != null ){
                    break;
                }
            }
        }
        return retProcess;
    }

    public ProcessDefinition getProcess(SatelliteLocation location,
                                        String strGroupID,
                                        String strProcessID ){
        ProcessDefinition retProcess = null;
        ProcessGroup group = ProcessManager.getInstance().getProcessGroup(location, strGroupID);
        Collection colProcesses = group.getProcesses();
        for( Iterator iProcesses = colProcesses.iterator(); iProcesses.hasNext(); ){
            ProcessDefinition process = (ProcessDefinition)iProcesses.next();
            if( process.getName().equals(strProcessID) ){
                retProcess = process;
                break;
            }
        }
        return retProcess;
    }

    /**
     * Dependents are services that require that this service is running
     * before they can execute properly.
     */
    public Collection getProcessDependents( SatelliteLocation location, String strProcessID ){
        return getProcessDependents( location, strProcessID, null );
    }

    public Collection getProcessDependents( SatelliteLocation location, String strProcessID,
                                            Document doc  ){
        ArrayList dependentsList = new ArrayList();
        Collection processes = null;
        if( doc != null ){
            processes = getAllProcesses( location, doc );
        }else{
            processes = getAllProcesses( location);
        }
        if( processes != null){
            for( Iterator iProcessIterator = processes.iterator(); iProcessIterator.hasNext(); ){
                ProcessDefinition processDef = (ProcessDefinition)iProcessIterator.next();
                if( ! processDef.getName().equals(strProcessID) ){
                    if( processDef.getDependencies().contains( strProcessID ) ){
                        dependentsList.add( processDef.getName() );
                    }
                }
            }
        }
        return dependentsList;

    }

    /**
     * handles the action of starting, stopping and restarting
     * a process with the processID strProcessID in the location "location".
     */
    public void handleProcessAction( SatelliteLocation location,
                                String strProcessID, String strCommand){

        try{
            String strURLToConnect = location.getAgentURL()
                +"?command="+strCommand+"&process="+XMLSupport.encodeUtf8(strProcessID) ;
            System.out.println("url to connect is " + strURLToConnect );
            URL agentURL = new URL(strURLToConnect);
            HttpURLConnection urlConn = (HttpURLConnection)agentURL.openConnection();
            urlConn.setDoOutput(true);
            urlConn.setRequestMethod("GET");
            urlConn.connect();
            InputStream inputStream = urlConn.getInputStream();
         }catch( IOException e ){
            //needs to be more specific.
            e.printStackTrace();
        }
    }

    public void handleGroupAction( SatelliteLocation location,
                                String strGroupID, String strCommand){

        try{
            Collection colProcesses = null;
            StringBuffer strBufURLToConnect =
                new StringBuffer(location.getAgentURL()+"?command="+strCommand);
            if( strGroupID.length() == 0 ){
                //get all processes in this location.
                 colProcesses = getAllProcesses( location );
            }else{
                ProcessGroup group = getProcessGroup( location, strGroupID);
                if( group != null ){
                    colProcesses = group.getProcesses();
                }
            }
            if( colProcesses != null ){
                for( Iterator procIterator=colProcesses.iterator(); procIterator.hasNext(); ){
                    ProcessDefinition process = (ProcessDefinition)procIterator.next();
                    if( strCommand.equals("start") ){
                        if(    process.getStatus().equals("Stopped")
                            || process.getStatus().equals("Failed") ){
                            strBufURLToConnect.append("&process="+XMLSupport.encodeUtf8(process.getName()));
                        }
                    }else if( strCommand.equals("stop") ){
                        if( process.getStatus().equals("Started")){
                            strBufURLToConnect.append("&process="+XMLSupport.encodeUtf8(process.getName()));
                        }
                    }else if( strCommand.equals("restart") ){
                        if( process.getStatus().equals("Started")){
                            strBufURLToConnect.append("&process="+XMLSupport.encodeUtf8(process.getName()));
                        }
                    }
                }
                System.out.println("url to connect is " + strBufURLToConnect.toString() );
                URL agentURL = new URL(strBufURLToConnect.toString());
                HttpURLConnection urlConn = (HttpURLConnection)agentURL.openConnection();
                urlConn.setDoOutput(true);
                urlConn.setRequestMethod("GET");
                urlConn.connect();
                InputStream inputStream = urlConn.getInputStream();
            }
         }catch( IOException e ){
            //needs to be more specific.
            e.printStackTrace();
        }
    }
    
    
    /**
     * Restarts all comm servers and media servers. 
     * Does not restart configuration server.
     * This is called after changes to config.defs
     */
    public void restartServers()
    {
        // TODO: implement restart servers function
    }

    public static void main( String args[] ){
        Hashtable hash = ProcessManager.getInstance().getAllProcessGroups();
        Enumeration enumeration = hash.keys();
        PrintStream out = System.out;
        while( enumeration.hasMoreElements() ){
             SatelliteLocation location = (SatelliteLocation)enumeration.nextElement();
             out.println(" location agent URL is " + location.getAgentURL() );
             Collection processGroups = (Collection)hash.get( location );
             Iterator iterator = processGroups.iterator();
             while( iterator.hasNext() ){
                ProcessGroup processGroup = (ProcessGroup)iterator.next();
                out.println(" process=" + processGroup);
             }
        }
    }

//////////////////////////////////////////////////////////////////////////////
// Implementation Methods
////

    /**
     *
     */
     private Collection getProcessGroups(InputStream inputStream){
       Collection retCol = null;
       Document doc = null;
       try {
            DOMBuilder domBuilder = new DOMBuilder();
            org.jdom.adapters.XercesDOMAdapter cdoma = 
                new org.jdom.adapters.XercesDOMAdapter();
            doc = domBuilder.build( cdoma.getDocument(inputStream, false ) );
       } catch ( JDOMException ex ) {
            ex.printStackTrace();
       } catch ( java.io.IOException ex ) {
           ex.printStackTrace();
       }
       if( doc != null )
            retCol =getProcessGroups( doc );
        return retCol;
     }

    private Collection getProcessGroups(Document doc){
        ArrayList groupList = new ArrayList();
        Element root = doc.getRootElement();
        Collection groups = root.getChildren( "group" );
        for(  Iterator iGroups = groups.iterator(); iGroups.hasNext(); ) {
            Element elGroup      = (Element) iGroups.next();
            String  strGroupName      = elGroup.getAttribute( "name" ).getValue();
            Collection colProcessesElements = elGroup.getChildren("process");
            ArrayList processList = new ArrayList();
            for( Iterator iProcesses = colProcessesElements.iterator(); iProcesses.hasNext(); ){
                Element elProcess = (Element)iProcesses.next();
                String  strName    = elProcess.getAttribute( "name" ).getValue();
                String  strStatus  = elProcess.getAttribute( "status" ).getValue();
                Collection colPIDElements = elProcess.getChildren("pid");
                String  strPID = "-1";
                for( Iterator ipid = colPIDElements.iterator(); ipid.hasNext(); ){
                    Element elPID = (Element)ipid.next();
                    strPID     = elPID.getText();
                }

                Collection colDependenciesElements = elProcess.getChildren("dependency");
                ArrayList colStrDependencies = new ArrayList();
                for( Iterator iDependencies= colDependenciesElements.iterator(); iDependencies.hasNext(); ){
                    Element elDependency = (Element)iDependencies.next();
                    String strDependency     = elDependency.getText().trim();
                    colStrDependencies.add( strDependency );
                }

                ProcessDefinition process =
                    new ProcessDefinition
                        ( strName, strGroupName, strStatus, strPID, colStrDependencies );
                processList.add( process );
            }
            ProcessGroup processGroup = new ProcessGroup( strGroupName, processList );
            groupList.add( processGroup );
        }

        return groupList;
    }





//////////////////////////////////////////////////////////////////////////////
// Nested / Inner classes
////



//////////////////////////////////////////////////////////////////////////////
// Native Method Declarations
////


}
