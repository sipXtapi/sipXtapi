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
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.pgs.user.ApplicationGroupAdvocate;
import com.pingtel.pds.pgs.user.ApplicationGroupAdvocateHome;
import org.apache.log4j.Category;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;


public class UnassignApplicationSetFromUserTag extends ExTagSupport {

    private String m_applicationGroupID;
    private String m_userID;

    private Category logger = Category.getInstance( "pgs" );

    private ApplicationGroupAdvocateHome m_agaHome = null;

    public void setUserid ( String userid ) {
        m_userID = userid;
    }

    public void setApplicationsetid ( String applicationsetid ){
        m_applicationGroupID = applicationsetid;
    }


    public int doStartTag() throws JspException {
        try {
            ApplicationGroupAdvocate advocate = null;
            if ( m_agaHome == null ) {
                m_agaHome = ( ApplicationGroupAdvocateHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  ApplicationGroupAdvocateHome.class,
                                                                    "ApplicationGroupAdvocate" );
            }

            advocate = m_agaHome.create();

            logger.debug( "doing unassign for user: " +
                m_userID + " app group: " + m_applicationGroupID );

            advocate.unassignUserAndApplicationGroup(   m_userID,
                                                            m_applicationGroupID );
            logger.debug( "all done" );
        }
        catch (Exception ex ) {
            throw new JspTagException( ex.getMessage() );
        }

        return SKIP_BODY;
    }



    protected void clearProperties() {
        m_applicationGroupID = null;
        m_userID = null;

        super.clearProperties();
    }
}
