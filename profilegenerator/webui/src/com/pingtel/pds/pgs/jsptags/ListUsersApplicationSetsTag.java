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
import com.pingtel.pds.pgs.jsptags.util.StyleTagSupport;
import com.pingtel.pds.pgs.user.Application;
import com.pingtel.pds.pgs.user.ApplicationGroup;
import com.pingtel.pds.pgs.user.ApplicationGroupHome;
import com.pingtel.pds.pgs.user.ApplicationHome;
import org.jdom.Element;

import javax.servlet.jsp.JspException;
import java.util.Collection;
import java.util.Iterator;


public class ListUsersApplicationSetsTag extends StyleTagSupport {

    private ApplicationGroupHome m_agHome = null;
    private ApplicationHome m_appHome = null;

    private String m_userID = null;

    public void setUserid ( String userid ) {
        m_userID = userid;
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

                m_appHome = ( ApplicationHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  ApplicationHome.class,
                                                                    "Application" );

            }

            Element documentRootElement = new Element("items");

            Collection items = m_agHome.findByUserID( m_userID );

            // Add each one of these to the document root element and post process it
            for (Iterator iter = items.iterator(); iter.hasNext(); ) {
                ApplicationGroup ag = ( ApplicationGroup ) iter.next();
                if ( !ag.getName().startsWith( "u" ) )
                {
                    Element nextElement =
                        createElementFromObject("applicationset", ag);

                    Collection applications =
                        m_appHome.findByApplicationGroupID( ag.getID() );

                    for ( Iterator iApp = applications.iterator(); iApp.hasNext(); ) {
                        Application app = (Application) iApp.next();
                        Element appElem = createElementFromObject("application", app );
                        nextElement.addContent( appElem );
                    }

                    documentRootElement.addContent( nextElement );
                }
            }

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
        m_userID = null;

        super.clearProperties();
    }
}