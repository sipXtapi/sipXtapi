/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/SetAdminPasswordTag.java#4 $
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
import com.pingtel.pds.pgs.user.User;
import com.pingtel.pds.pgs.user.UserAdvocate;
import com.pingtel.pds.pgs.user.UserAdvocateHome;
import com.pingtel.pds.pgs.user.UserHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;



public class SetAdminPasswordTag extends ExTagSupport {

    private String m_userID = null;
    private String m_password = null;

    private UserHome m_userHome = null;
    private UserAdvocate m_userAdvocate = null;

    public void setUserid( String userid ){
        m_userID = userid;
    }

    public void setPassword ( String password ){
        m_password = password;
    }


    public int doStartTag() throws JspException {
        try {
            if ( m_userHome == null ) {
                m_userHome = ( UserHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  UserHome.class,
                                                                    "User" );

                UserAdvocateHome userAdvocateHome = (UserAdvocateHome)
                    EJBHomeFactory.getInstance().getHomeInterface(  UserAdvocateHome.class,
                                                                    "UserAdvocate" );

                m_userAdvocate = userAdvocateHome.create();
            }

            User user = m_userHome.findByPrimaryKey( m_userID );
            String displayID = user.getDisplayID();

            m_userAdvocate.setUsersPassword( m_userID, displayID, m_password );
        }
        catch (Exception ex ) {
            throw new JspTagException( ex.getMessage() );
        }

        return SKIP_BODY;
    }



    protected void clearProperties() {
        m_userID = null;
        m_password = null;

        super.clearProperties();
    }
}
