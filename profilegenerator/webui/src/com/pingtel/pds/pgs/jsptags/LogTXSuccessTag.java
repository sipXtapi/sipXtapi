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

import javax.servlet.jsp.JspException;

import org.apache.log4j.Category;
import org.apache.log4j.NDC;

import com.pingtel.pds.pgs.jsptags.util.ExTagSupport;
import com.pingtel.pds.common.TXPriority;

public class LogTXSuccessTag extends ExTagSupport {

    private String m_userID = null;
    private String m_message = null;
    private String m_organizationName = null;

    public void setUserid( String userid ) {
        m_userID = userid;
    }

    public void setMessage ( String message ) {
        m_message = message;
    }

    public void setOrgname ( String orgname ) {
        m_organizationName = orgname;
    }


    public int doStartTag() throws JspException {
        Category logger = Category.getInstance ("pgs");

        NDC.clear();
        NDC.push( "Organization: " + m_organizationName );
        NDC.push( "UserID: " + m_userID );

        logger.log( TXPriority.TX_MESSAGE, (String) m_message );

        return SKIP_BODY;
    }

    protected void clearProperties() {
        super.clearProperties();

        m_userID = null;
        m_message = null;
        m_organizationName = null;
    }
}
