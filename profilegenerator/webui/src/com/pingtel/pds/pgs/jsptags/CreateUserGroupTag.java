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
import com.pingtel.pds.common.RedirectServletException;
import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.pgs.user.UserGroup;
import com.pingtel.pds.pgs.user.UserGroupAdvocate;
import com.pingtel.pds.pgs.user.UserGroupAdvocateHome;

import javax.servlet.jsp.JspException;

public class CreateUserGroupTag extends ExTagSupport {

    private String m_userGroupName = null;
    private String m_parentGroupID = null;
    private String m_refConfigSetID = null;
    private String m_parameterString = null;

    private UserGroupAdvocateHome m_userGroupHome = null;
    private UserGroupAdvocate m_userGroupAdvocate = null;

    public void setUsergroupname( String usergroupname ) {
        m_userGroupName = usergroupname;
    }

    public void setParentgroupid( String parentgroupid ) {
        m_parentGroupID = parentgroupid;
    }

    public void setRefconfigsetid( String refconfigsetid ) {
        m_refConfigSetID = refconfigsetid;
    }

    public void setParameterstring ( String parameterstring ) {
        m_parameterString = parameterstring;
    }


    public int doStartTag() throws JspException {
        try {
            if ( m_userGroupHome == null ) {
                m_userGroupHome = ( UserGroupAdvocateHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  UserGroupAdvocateHome.class,
                                                                    "UserGroupAdvocate" );

                m_userGroupAdvocate = m_userGroupHome.create();
            }

            UserGroup ug = m_userGroupAdvocate.createUserGroup( m_userGroupName,
                                                               m_parentGroupID,
                                                               m_refConfigSetID,
                                                               getOrganizationID() );
        }
        catch (Exception ex ) {
            throw new RedirectServletException (    ex.getMessage(),
                                                    "../popup/form_add_usergroup.jsp",
                                                    m_parameterString );
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        m_userGroupName = null;
        m_parentGroupID = null;
        m_refConfigSetID = null;

        super.clearProperties();
    }
}
