/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/EditUserGroupTag.java#4 $
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

public class EditUserGroupTag extends ExTagSupport {

    private UserGroupAdvocateHome ugHome = null;
    private String m_userGroupID;
    private String m_userGroupName;
    private String m_parentGroupID;
    private String m_refConfigSetID;

    public void setUsergroupid( String usergroupid ) {
      m_userGroupID = usergroupid;
    }

    public void setUsergroupname( String usergroupname ) {
      m_userGroupName = usergroupname;
    }

    public void setParentgroupid( String parentgroupid ) {
      m_parentGroupID = parentgroupid;
    }

    public void setRefconfigsetid( String refconfigsetid ) {
      m_refConfigSetID = refconfigsetid;
    }

    public int doStartTag() throws JspException {
        try {
            if ( ugHome == null ) {
                ugHome = ( UserGroupAdvocateHome )
                        EJBHomeFactory.getInstance().getHomeInterface(  UserGroupAdvocateHome.class,
                                "UserGroupAdvocate" );
            }

            UserGroupAdvocate ugAdvocate = ugHome.create();

            ugAdvocate.editUserGroup(   m_userGroupID,
                    m_userGroupName,
                    m_parentGroupID,
                    m_refConfigSetID );
        }
        catch (Exception ex ) {
            throw new JspTagException( ex.getMessage() );
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        m_userGroupID = null;
        m_userGroupName = null;
        m_parentGroupID = null;
        m_refConfigSetID = null;

        super.clearProperties();
    }
}
