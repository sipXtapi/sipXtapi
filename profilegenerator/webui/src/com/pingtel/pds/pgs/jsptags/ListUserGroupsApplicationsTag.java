/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/ListUserGroupsApplicationsTag.java#5 $
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
import java.util.StringTokenizer;


public class ListUserGroupsApplicationsTag extends StyleTagSupport {

    private ApplicationHome m_applicationHome = null;
    private ApplicationGroupHome m_agHome = null;

    private Integer m_userGroupID;

    public void setUsergroupid ( String usergroupid ) {
        this.m_userGroupID = new Integer ( usergroupid );
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

            Element documentRootElement = new Element("items");

            Collection allAG = m_agHome.findByUserGroupID( m_userGroupID );
            for ( Iterator iAll = allAG.iterator(); iAll.hasNext(); ) {
                ApplicationGroup ag = (ApplicationGroup) iAll.next();
                if ( ag.getName().startsWith( "ug" + m_userGroupID.toString() ) ) {
                    StringTokenizer st = new StringTokenizer ( ag.getName(), "_" );
                    String userGroup = st.nextToken();
                    String appID = st.nextToken();
                    Application app =
                        m_applicationHome.findByPrimaryKey( Integer.valueOf( appID ) );

                    Element appElement = createElementFromObject( "application", app );
                    documentRootElement.addContent( appElement );
                }
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
        m_userGroupID = null;

        super.clearProperties();
    }
}
