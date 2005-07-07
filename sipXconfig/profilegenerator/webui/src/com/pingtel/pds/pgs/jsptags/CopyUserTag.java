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
import com.pingtel.pds.pgs.user.User;
import com.pingtel.pds.pgs.user.UserAdvocate;
import com.pingtel.pds.pgs.user.UserAdvocateHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;




public class CopyUserTag extends ExTagSupport {

    private String m_sourceUserID = null;

    private UserAdvocateHome uaHome = null;
    private UserAdvocate uAdvocate = null;

    public void setSourceuserid( String sourceuserid ){
        m_sourceUserID = sourceuserid;
    }


    /**
     *
     * @return
     * @exception JspException
     */
    public int doStartTag() throws JspException {
        try {
            if ( uaHome == null ) {
                uaHome = ( UserAdvocateHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  UserAdvocateHome.class,
                                                                    "UserAdvocate" );

                uAdvocate = uaHome.create();
            }

            User user = uAdvocate.copyUser( this.m_sourceUserID );

        }
        catch (Exception ex ) {
            throw new JspTagException( ex.getMessage() );
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        m_sourceUserID = null;

        super.clearProperties();
    }
}
