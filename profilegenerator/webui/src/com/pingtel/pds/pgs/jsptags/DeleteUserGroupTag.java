/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/DeleteUserGroupTag.java#4 $
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
import com.pingtel.pds.pgs.user.UserGroupAdvocate;
import com.pingtel.pds.pgs.user.UserGroupAdvocateHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;

public class DeleteUserGroupTag extends ExTagSupport {

    private String m_userGroupID = null;
    private String m_option = null;
    private String m_newUserGroupID = null;
    private UserGroupAdvocateHome ugHome = null;
    private UserGroupAdvocate ugAdvocate = null;

    public void setUsergroupid( String usergroupid ) {
        m_userGroupID = usergroupid;
    }

    public void setOption( String option ) {
        m_option = option;
    }

    public void setNewusergroupid( String newusergroupid ) {
        m_newUserGroupID = newusergroupid;
    }

    public int doStartTag() throws JspException {
        try {

            if ( ugHome == null ) {
                ugHome = ( UserGroupAdvocateHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  UserGroupAdvocateHome.class,
                                                                    "UserGroupAdvocate" );

                ugAdvocate = ugHome.create();
            }

            ugAdvocate.deleteUserGroup( m_userGroupID, m_option, m_newUserGroupID );
        }
        catch (Exception ex ) {
            throw new JspTagException( ex.getMessage() );
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        m_userGroupID = null;
        m_option = null;
        m_newUserGroupID = null;

        super.clearProperties();
    }
}
