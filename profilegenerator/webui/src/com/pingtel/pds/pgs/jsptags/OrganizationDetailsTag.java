/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/OrganizationDetailsTag.java#4 $
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
import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;
import com.pingtel.pds.pgs.organization.Organization;
import com.pingtel.pds.pgs.organization.OrganizationHome;
import org.jdom.Element;

import javax.servlet.jsp.JspException;


public class OrganizationDetailsTag extends StyleTagSupport {
    private OrganizationHome m_organizationHome = null;


    public int doStartTag() throws JspException {
        try {

            if ( m_organizationHome == null ) {
                m_organizationHome = ( OrganizationHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  OrganizationHome.class,
                                                                    "Organization" );
            }

            Element documentRootElement = new Element("items");
            // Find the unordered collection of EJB objects
            //Collection items = m_organizationHome.findByOrganizationID( new Integer ( getOrganizationID() ) );
            Organization o = m_organizationHome.findByPrimaryKey( new Integer ( this.getOrganizationID() ) );

            Element nextElement = createElementFromObject("item", o );

            documentRootElement.addContent( nextElement );

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
     * Called by super class method (not by superClass's clearProperties
     * method).
     */
    protected void clearProperties() {

        super.clearProperties();
    }
}