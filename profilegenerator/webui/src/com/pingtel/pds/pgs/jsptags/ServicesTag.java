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

package com.pingtel.pds.pgs.jsptags;

import java.io.IOException;
import java.rmi.RemoteException;
import java.util.Collection;
import java.util.Iterator;
import java.net.URLEncoder;

import javax.servlet.jsp.JspException;

import org.jdom.Document;
import org.jdom.Element;
import org.jdom.CDATA;
import org.jdom.JDOMException;
import org.jdom.input.SAXBuilder;

import com.pingtel.pds.common.XMLSupport;
import com.pingtel.pds.common.ElementUtilException;
import com.pingtel.pds.common.PostProcessingException;
import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;


import com.pingtel.pds.pgs.sipxchange.*;
import com.pingtel.pds.pgs.sipxchange.process.*;
import java.io.InputStream;
/**
 * <p>Title: ServicesTag</p>
 * <p>Description: Tag Library for the DMS JSP Pages</p>
 * <p>Copyright: Copyright (c) 2002</p>
 * <p>Company: Pingtel Corp</p>
 * @author Pradeep Paudyal
 * @version 1.0
 */

public class ServicesTag extends StyleTagSupport {

    private String m_strCommand    = null;
    private String m_strProcessID  = null;
    private String m_strLocationID = null;
    private String m_strGroupID    = null;
    private String m_strRedirect   = null;


    public void setLocation( String location ) {
        m_strLocationID = location;
    }


    public void setProcess( String process ) {
        m_strProcessID = process;
    }

    public void setCommand( String command ) {
        m_strCommand = command;
    }

    public void setGroup( String group ) {
        m_strGroupID = group;
    }

    public void setRedirect( String redirect ) {
        m_strRedirect = redirect;
    }

    /**
     * This list all the devices here
     */
    public int doStartTag() throws JspException {

        SatelliteLocation location = null;
        if(m_strLocationID != null){
             location =
                SatelliteManager.getInstance().getLocation( m_strLocationID );
        }
        if(m_strCommand    != null){
             if(location != null){
               //if not, if it is for group, handle start/stop/restart
                //for groups
                //if it is a process , handle start/stop/restart for
                //a process.
                if( m_strProcessID  != null ){
                    ProcessManager.getInstance().handleProcessAction
                        ( location, m_strProcessID, m_strCommand);
                }else if( m_strGroupID  != null ) {
                    ProcessManager.getInstance().handleGroupAction
                        ( location, m_strGroupID, m_strCommand);
                }

             }
        }
        if( m_strRedirect != null ){
            if( m_strRedirect.equals("list") ){
                listServices();
            }else if(  m_strRedirect.equals("details") ){
                 if( location != null ){
                    showDetails( location, m_strProcessID);
                 }
            }
        }else{
            listServices();
        }


        return SKIP_BODY;
    }

     /**
      * Post Process the XML and insert friendly sub elements where
      * we have primmary keys
      */
    protected Element postProcessElement( Element inputElement )
        throws PostProcessingException {
        // @JC TODO
        return inputElement;
    }

    /**
     * Called by super class method (not by superClass's clearProperties
     * method).
     */
    protected void clearProperties() {
        super.clearProperties();
    }



    private void showDetails( SatelliteLocation location, String strProcessID)
                                                             throws JspException{
        // Sends the text to the browser in either xml or html format
        // depending on an optional debug tag attribute

         //just do one for now.
        Document rootDocument = ProcessManager.getInstance().getProcessInfoDocument(location) ;
        Document rootDocumentClone1 = (Document)rootDocument.clone();
        Document rootDocumentClone2 = (Document)rootDocument.clone();

        if( rootDocument != null ){

            ProcessDefinition process = ProcessManager.getInstance().getProcess
                                            ( location, strProcessID, rootDocument);
            if( process != null ){
            try{
                //Element documentRootElement = new Element("process");

                Element processElement =
                    ProcessManager.getInstance()
                            .getProcessDefinitionsData(location, strProcessID, rootDocumentClone1) ;
                if( processElement != null ){

                    processElement.setAttribute("location_id",   location.getLocationID());
                    processElement.setAttribute("location_hostname",   location.getHostName());
                    processElement.setAttribute("group",  process.getGroup());
                    processElement.setAttribute("encodedName",
                        XMLSupport.encodeUtf8(process.getName()));
                    processElement.setAttribute("encodedLocationID",
                        XMLSupport.encodeUtf8(location.getLocationID()));
                    processElement.setAttribute("encodedGroupName",
                        XMLSupport.encodeUtf8(process.getGroup()));

                    Collection colDependenciesElements = processElement.getChildren("dependency");
                    for( Iterator iDependencies= colDependenciesElements.iterator(); iDependencies.hasNext(); ){
                        Element elDependency = (Element)iDependencies.next();
                        elDependency.setAttribute("encodedName", XMLSupport.encodeUtf8(elDependency.getText().trim()));
                    }

                    if( (colDependenciesElements == null) || (colDependenciesElements.size() == 0) ){
                        processElement.setAttribute("dependencies_size", "0");
                    }else{
                        processElement.setAttribute("dependencies_size", colDependenciesElements.size()+"");
                    }

                    Collection colDependents = ProcessManager.getInstance()
                        .getProcessDependents( location, process.getName(), rootDocumentClone2);
                    for( Iterator iDependents = colDependents.iterator();iDependents.hasNext(); ){
                        String strDependent = (String) iDependents.next();
                        Element dependentElement = new Element( "dependent" );
                        dependentElement.setAttribute("name", strDependent);
                        dependentElement.setAttribute("encodedName", XMLSupport.encodeUtf8(strDependent));
                        processElement.addContent( dependentElement );
                    }
                    if( (colDependents == null) || (colDependents.size() == 0) ){
                        processElement.setAttribute("dependents_size", "0");
                    }else{
                        processElement.setAttribute("dependents_size", colDependents.size()+"");
                    }
                    outputTextToBrowser ( processElement );
                }
            } catch( ElementUtilException ex ) {
               throw new JspException( ex.getMessage());
            }catch( IOException ex ){
               throw new JspException( ex.getMessage());
            }
            }
        }
    }

    /**
     * list all services in all the hosts.
     */
    private void listServices() throws JspException {
        // Sends the text to the browser in either xml or html format
        // depending on an optional debug tag attribute
         Iterator iterator = SatelliteManager.getInstance().getAllLocations().iterator();

         //just do one for now.
        try{
             Element documentRootElement = new Element("items");

             while( iterator.hasNext() ) {
                    SatelliteLocation location =
                                    (SatelliteLocation)iterator.next();

                    Element hostElement = new Element("host");
                    hostElement.setAttribute("id",       location.getLocationID());
                    hostElement.setAttribute("encodedID", XMLSupport.encodeUtf8(
                                     location.getLocationID()));
                    String strHostName = location.getHostName();
                    if( strHostName == null )
                        strHostName = "";
                    hostElement.setAttribute("name",     strHostName);
                    hostElement.setAttribute("status",   "unavailable");
                    InputStream inputStream = ProcessManager.getInstance().getProcessInfoXMLData(location) ;
                    if( inputStream != null ){
                        hostElement.setAttribute("status",   "available");
                        SAXBuilder saxBuilder = new SAXBuilder();
                        Document doc = saxBuilder.build( inputStream );
                        Element processRootElement = (Element)(doc.getRootElement().clone());

                        Collection groups = processRootElement.getChildren( "group" );
                        for ( Iterator iGroups = groups.iterator(); iGroups.hasNext(); ) {
                             Element group = (Element) iGroups.next();
                             group.setAttribute("encodedName", XMLSupport.encodeUtf8(
                                     group.getAttribute("name").getValue()));

                             group.setAttribute("start",   "disable");
                             group.setAttribute("stop",    "disable");
                             group.setAttribute("restart", "disable");

                             Collection processes = group.getChildren( "process" );
                             for ( Iterator iProcesses = processes.iterator(); iProcesses.hasNext(); ) {
                                Element process = (Element) iProcesses.next();
                                if( process.getAttribute("status").getValue().equals("Started") ){
                                    group.setAttribute("stop",   "enable");
                                    group.setAttribute("restart", "enable");
                                }else if(  process.getAttribute("status").getValue().equals("Stopped")
                                        || process.getAttribute("status").getValue().equals("Failed")){
                                    group.setAttribute("start", "enable");
                                }
                                process.setAttribute("encodedName", XMLSupport.encodeUtf8(
                                     process.getAttribute("name").getValue()));
                             }

                         }
                         hostElement.setAttribute("start",   "disable");
                         hostElement.setAttribute("stop",    "disable");
                         hostElement.setAttribute("restart", "disable");
                         for ( Iterator iGroups = groups.iterator(); iGroups.hasNext(); ) {
                             Element group = (Element) iGroups.next();
                              if( group.getAttribute("start").getValue().equals("enable") ){
                                    hostElement.setAttribute("start",   "enable");
                              }
                              if( group.getAttribute("stop").getValue().equals("enable") ){
                                    hostElement.setAttribute("stop", "enable");
                              }
                              if( group.getAttribute("restart").getValue().equals("enable") ){
                                    hostElement.setAttribute("restart", "enable");
                              }
                         }
                         hostElement.addContent(processRootElement);
                    }
                    documentRootElement.addContent( hostElement );


             }
             outputTextToBrowser ( documentRootElement );

        } catch ( JDOMException ex ) {
            ex.printStackTrace();
        } catch( ElementUtilException ex ) {
             throw new JspException( ex.getMessage());
        } catch( IOException ex ){
             throw new JspException( ex.getMessage());
        }
    }


}
