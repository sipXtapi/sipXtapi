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
import com.pingtel.pds.pgs.user.UserGroupAdvocate;
import com.pingtel.pds.pgs.user.UserGroupAdvocateHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;

public class CopyUserGroupTag extends ExTagSupport {

    private String m_sourceUserGroupID = null;

    private UserGroupAdvocateHome ugHome = null;
    private UserGroupAdvocate ugAdvocate = null;

    public void setSourceusergroupid( String sourceusergroupid ) {
        m_sourceUserGroupID = sourceusergroupid;
    }


    public int doStartTag() throws JspException {
        try {
            if ( ugHome == null ) {
                ugHome = ( UserGroupAdvocateHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  UserGroupAdvocateHome.class,
                                                                    "UserGroupAdvocate" );

                ugAdvocate = ugHome.create();
            }

            ugAdvocate.copyUserGroup( m_sourceUserGroupID );
        }
        catch (Exception ex ) {
            throw new JspTagException( ex.getMessage() );
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        m_sourceUserGroupID = null;

        super.clearProperties();
    }
}
