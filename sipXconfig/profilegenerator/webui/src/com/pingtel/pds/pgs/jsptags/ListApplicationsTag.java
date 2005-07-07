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
import com.pingtel.pds.pgs.user.Application;
import com.pingtel.pds.pgs.user.ApplicationGroup;
import com.pingtel.pds.pgs.user.ApplicationGroupHome;
import com.pingtel.pds.pgs.user.ApplicationHome;
import org.jdom.Element;

import javax.servlet.jsp.JspException;
import java.util.Collection;
import java.util.Iterator;


public class ListApplicationsTag extends StyleTagSupport {

    private ApplicationHome m_applicationHome = null;
    private ApplicationGroupHome m_agHome = null;

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

            Element documentRootElement = new Element("items");

            Collection apps =
                m_applicationHome.findByOrganizationID( new Integer ( this.getOrganizationID() ) );

            for ( Iterator iA = apps.iterator(); iA.hasNext(); ) {
                Application app = (Application) iA.next();

                Element nextElement =  createElementFromObject("item", app);

                documentRootElement.addContent( nextElement );

                Collection appGroups =
                                m_agHome.findByApplicationID( app.getID() );

                Element applicationGroupsElement =
                    new Element ( "applicationsets" );

                nextElement.addContent( applicationGroupsElement );

                for ( Iterator iAG = appGroups.iterator(); iAG.hasNext(); ) {
                    ApplicationGroup ag = (ApplicationGroup) iAG.next();

                    Element agElement =
                        createElementFromObject ( "applicationset", ag );

                    applicationGroupsElement.addContent( agElement );
                }
            } // for all application groups

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
        super.clearProperties();
    }
}
