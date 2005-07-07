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
import com.pingtel.pds.pgs.user.ApplicationAdvocate;
import com.pingtel.pds.pgs.user.ApplicationAdvocateHome;
import org.apache.log4j.Category;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;


public class AssignApplicationToUserGroupTag extends ExTagSupport {

    private String m_userGroupID;
    private String m_applicationID;

    private Category logger = Category.getInstance( "pgs" );

    private ApplicationAdvocateHome m_aaHome = null;

    public void setApplicationid ( String applicationid ) {
        m_applicationID = applicationid;
    }

    public void setUsergroupid ( String usergroupid ){
        m_userGroupID = usergroupid;
    }


    public int doStartTag() throws JspException {
        try {
            ApplicationAdvocate advocate = null;
            if ( m_aaHome == null ) {
                m_aaHome = (ApplicationAdvocateHome)
                    EJBHomeFactory.getInstance().getHomeInterface(  ApplicationAdvocateHome.class,
                                                                    "ApplicationAdvocate" );
            }

            advocate = m_aaHome.create();

            logger.debug( "doing assign for user group: " + m_userGroupID + " app: " + m_applicationID );
            advocate.assignApplicationToUserGroup( m_userGroupID, m_applicationID );
            logger.debug( "all done" );

        }
        catch (Exception ex ) {
            throw new JspTagException( ex.getMessage() );
        }

        return SKIP_BODY;
    }


    protected void clearProperties() {
        m_userGroupID = null;
        m_applicationID = null;

        super.clearProperties();
    }
}
