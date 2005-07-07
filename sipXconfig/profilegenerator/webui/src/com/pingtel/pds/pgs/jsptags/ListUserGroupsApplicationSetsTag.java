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

import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.common.PostProcessingException;
import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;
import com.pingtel.pds.pgs.user.ApplicationGroup;
import com.pingtel.pds.pgs.user.ApplicationGroupHome;
import org.jdom.Element;

import javax.servlet.jsp.JspException;
import java.util.Collection;
import java.util.Iterator;


public class ListUserGroupsApplicationSetsTag extends StyleTagSupport {
    private ApplicationGroupHome m_agHome = null;

    private Integer m_userGroupID = null;

    public void setUsergroupid ( String usergroupid ) {
        m_userGroupID = new Integer ( usergroupid );
    }

    /**
     * This list all the devices here
     */
    public int doStartTag() throws JspException {
        try {
            // OPtimization to look up all home interfaces here
            if ( m_agHome == null ) {
                m_agHome = ( ApplicationGroupHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  ApplicationGroupHome.class,
                                                                    "ApplicationGroup" );
            }

            Element documentRootElement = new Element("items");

            Collection items = m_agHome.findByUserGroupID( m_userGroupID );

            // Add each one of these to the document root element and post process it
            for (Iterator iter = items.iterator(); iter.hasNext(); ) {
                ApplicationGroup ag = ( ApplicationGroup ) iter.next();
                if ( !ag.getName().startsWith( "ug" ) )
                {
                    Element nextElement =
                        createElementFromObject("applicationset", ag);
                    Element fullyParsedElement = postProcessElement ( nextElement );
                    documentRootElement.addContent( fullyParsedElement );
                }
            }
            // Sends the text to the browser in either xml or html format
            // depending on an optional debug tag attribute
            outputTextToBrowser ( documentRootElement );
        }
        catch(Exception ex ) {
             throw new JspException(ex.getMessage());
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
}
