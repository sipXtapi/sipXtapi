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
import com.pingtel.pds.pgs.user.User;
import com.pingtel.pds.pgs.user.UserAdvocate;
import com.pingtel.pds.pgs.user.UserAdvocateHome;
import com.pingtel.pds.pgs.user.UserHome;
import org.jdom.Element;

import javax.servlet.jsp.JspException;
import java.util.Collection;
import java.util.Iterator;


public class AdministratorDetailsTag extends StyleTagSupport {

    private String m_userID = null;

    private UserHome m_userHome = null;
    private UserAdvocateHome m_uaHome = null;
    private UserAdvocate m_userAdvocate = null;

    public void setUserid ( String userid ) {
        this.m_userID = userid;
    }

    public int doStartTag() throws JspException {
        try {
            // OPtimization to look up all home interfaces here
            if ( m_userHome == null ) {
                m_userHome = (UserHome)
                    EJBHomeFactory.getInstance().getHomeInterface( UserHome.class, "User" );

                m_uaHome = (UserAdvocateHome)
                    EJBHomeFactory.getInstance().getHomeInterface( UserAdvocateHome.class, "UserAdvocate" );

                m_userAdvocate = m_uaHome.create();
            }

            //Elemenet root = new Element ( "Administrator" );

            User user = m_userHome.findByPrimaryKey( m_userID );
            Element preprocessed = createElementFromObject("user", user);

            Element fullyParsedElement = postProcessElement ( preprocessed );
            //root.addContent( fullyParsedElement );

            Element roles = new Element ( "roles" );

            Collection cRoles = m_userAdvocate.getSecurityRoles( user.getID() );

            for ( Iterator i = cRoles.iterator(); i.hasNext(); ) {
                String role = (String) i.next();

                Element roleElem = new Element ( "role" );
                roleElem.addContent( role );
                roles.addContent( roleElem );
            }

            fullyParsedElement.addContent( roles );

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
        super.clearProperties();
    }
}
