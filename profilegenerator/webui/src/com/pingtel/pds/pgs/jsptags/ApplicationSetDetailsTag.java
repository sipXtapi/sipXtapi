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


public class ApplicationSetDetailsTag extends StyleTagSupport {

    private String m_agID = null;

    private ApplicationGroupHome m_agHome = null;
    private ApplicationHome m_applicationHome = null;

    public void setApplicationsetid ( String applicationsetid ) {
        this.m_agID = applicationsetid;
    }

    public int doStartTag() throws JspException {
        try {
            if ( m_agHome == null ) {
                m_applicationHome =  (ApplicationHome)
                    EJBHomeFactory.getInstance().getHomeInterface( ApplicationHome.class, "Application" );

                m_agHome =  (ApplicationGroupHome)
                    EJBHomeFactory.getInstance().getHomeInterface( ApplicationGroupHome.class, "ApplicationGroup" );
            }

            ApplicationGroup ag = m_agHome.findByPrimaryKey( new Integer ( m_agID ) );
            Element preprocessed = createElementFromObject("applicationset", ag );

            Element fullyParsedElement = postProcessElement ( preprocessed );


            Element members = new Element ( "members" );

            Collection cMembers =
                m_applicationHome.findByApplicationGroupID( new Integer ( m_agID ) );

            for ( Iterator i = cMembers.iterator(); i.hasNext(); ) {
                Application application = (Application) i.next();
                Element appElem = createElementFromObject("application", application );
                members.addContent( appElem );
            }

            fullyParsedElement.addContent( members );

            outputTextToBrowser ( fullyParsedElement );
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
    protected Element postProcessElement( Element inputElement ) {

        return inputElement;
    }

    /**
     * Called by super class method (not by superClass's clearProperties
     * method).
     */
    protected void clearProperties() {
        m_agID = null;

        super.clearProperties();
    }
}