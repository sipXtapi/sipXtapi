/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/ListAdministratorsTag.java#5 $
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
import com.pingtel.pds.pgs.user.UserHome;
import org.jdom.Element;

import javax.servlet.jsp.JspException;
import java.util.Collection;
import java.util.Iterator;

/**
 * <p>Title: ListDevicesTag</p>
 * <p>Description: Tag Library for the DMS JSP Pages</p>
 * <p>Copyright: Copyright (c) 2002</p>
 * <p>Company: Pingtel Corp</p>
 * @author John P. Coffey
 * @version 1.0
 */

public class ListAdministratorsTag extends StyleTagSupport {
    private UserHome m_userHome = null;

    /**
     * This list all the devices here
     */
    public int doStartTag() throws JspException {
        try {
            // OPtimization to look up all home interfaces here
            if ( m_userHome == null ) {
                m_userHome = ( UserHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  UserHome.class,
                                                                    "User" );
            }

            Element documentRootElement = new Element("items");

            // Find the unordered collection of System Administrators
            Collection items = m_userHome.
                findAdministratorsByOrganizationID( new Integer ( getOrganizationID() ) );

            // Add each one of these to the document root element and post process it
            for (Iterator iter = items.iterator(); iter.hasNext(); ) {
                Object nextEJBObject = iter.next();
                Element nextElement =
                    createElementFromObject("item", nextEJBObject);
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
        super.clearProperties();
    }
}
