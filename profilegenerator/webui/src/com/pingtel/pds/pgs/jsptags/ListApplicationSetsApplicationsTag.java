/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/ListApplicationSetsApplicationsTag.java#5 $
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
import com.pingtel.pds.pgs.user.ApplicationHome;
import org.jdom.Element;

import javax.servlet.jsp.JspException;
import java.util.Collection;
import java.util.Iterator;


public class ListApplicationSetsApplicationsTag extends StyleTagSupport {
    private ApplicationHome m_applicationHome = null;

    private Integer m_applicationSetID = null;

    public void setApplicationsetid ( String applicationsetid ) {
        m_applicationSetID = new Integer ( applicationsetid );
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
            }

            Element documentRootElement = new Element("items");

            Collection items = m_applicationHome.findByApplicationGroupID( m_applicationSetID );

            // Add each one of these to the document root element and post process it
            for (Iterator iter = items.iterator(); iter.hasNext(); ) {
                Object nextEJBObject = iter.next();
                Element nextElement =
                    createElementFromObject("application", nextEJBObject);
                // Expand on any SubElements in the EJB Object
                // displaying friendly names etc.
                Element fullyParsedElement = postProcessElement ( nextElement );
                documentRootElement.addContent( fullyParsedElement );
            }
            // Sends the text to the browser in either xml or html format
            // depending on an optional debug tag attribute
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

        return inputElement;
    }

    /**
     * Called by super class method (not by superClass's clearProperties
     * method).
     */
    protected void clearProperties() {
        m_applicationSetID = null;

        super.clearProperties();
    }
}
