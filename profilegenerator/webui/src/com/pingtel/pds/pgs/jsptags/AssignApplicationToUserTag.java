/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/AssignApplicationToUserTag.java#4 $
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
import com.pingtel.pds.pgs.user.ApplicationAdvocate;
import com.pingtel.pds.pgs.user.ApplicationAdvocateHome;
import org.apache.log4j.Category;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;


public class AssignApplicationToUserTag extends ExTagSupport {

    private String m_userID;
    private String m_applicationID;

    private Category logger = Category.getInstance( "pgs" );

    private ApplicationAdvocateHome m_aaHome = null;

    public void setApplicationid ( String applicationid ) {
        m_applicationID = applicationid;
    }

    public void setUserid ( String userid ){
        m_userID = userid;
    }


    public int doStartTag() throws JspException {
        try {
            ApplicationAdvocate advocate = null;
            if ( m_aaHome == null ) {
                m_aaHome = (ApplicationAdvocateHome)
                    EJBHomeFactory.getInstance().getHomeInterface(  ApplicationAdvocateHome.class,
                                                                    "ApplicationAdvocate" );
            }

            logger.debug( "before advocate create" );

            advocate = m_aaHome.create();

            logger.debug( "doing assign for user: " + m_userID + " app: " + m_applicationID );


            advocate.assignApplicationToUser( m_userID, m_applicationID );
            logger.debug( "all done" );

        }
        catch (Exception ex ) {
            throw new JspTagException( ex.getMessage() );
        }

        return SKIP_BODY;
    }



    protected void clearProperties() {
        m_userID = null;
        m_applicationID = null;

        super.clearProperties();
    }
}
