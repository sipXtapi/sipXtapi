/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/ListAppSetsAvailApplicationsTag.java#5 $
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

import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.common.PostProcessingException;
import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;
import com.pingtel.pds.pgs.user.Application;
import com.pingtel.pds.pgs.user.ApplicationGroupHome;
import com.pingtel.pds.pgs.user.ApplicationHome;
import org.jdom.Element;

import javax.servlet.jsp.JspException;
import java.util.Collection;
import java.util.Iterator;


public class ListAppSetsAvailApplicationsTag extends StyleTagSupport {

    private ApplicationHome m_applicationHome = null;
    private ApplicationGroupHome m_agHome = null;

    private String m_appSetID;

    public void setApplicationsetid ( String applicationsetid ) {
        this.m_appSetID = applicationsetid;
    }

    /**
     * This list all the devices here
     */
    public int doStartTag() throws JspException {
        try {
            // OPtimization to look up all home interfaces here
            if ( m_applicationHome == null ) {
                m_applicationHome = ( ApplicationHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  ApplicationHome.class,
                                                                    "Application" );
                m_agHome = ( ApplicationGroupHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  ApplicationGroupHome.class,
                                                                    "ApplicationGroup" );
            }

            Collection allApps = m_applicationHome.findAll();

            Element documentRootElement = new Element("items");

            Collection existingApps =
                m_applicationHome.findByApplicationGroupID( Integer.valueOf( m_appSetID ) );

            for ( Iterator iAll = existingApps.iterator(); iAll.hasNext(); ) {
                Application app = (Application) iAll.next();

                if ( allApps.contains( app ) ) {
                    allApps.remove( app );
                }
            }

            for ( Iterator filteredI = allApps.iterator(); filteredI.hasNext(); ) {
                Application app = (Application) filteredI.next();

                Element appElement = createElementFromObject( "application", app );
                documentRootElement.addContent( appElement );
             }


            outputTextToBrowser ( documentRootElement );
        }
        catch(Exception ex ) {
             throw new JspException( ex.getMessage());
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
        m_appSetID = null;

        super.clearProperties();
    }
}
