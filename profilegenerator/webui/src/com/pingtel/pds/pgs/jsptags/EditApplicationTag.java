/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/EditApplicationTag.java#4 $
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

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;


public class EditApplicationTag extends ExTagSupport {

    private String m_applicationID;
    private String m_name;
    private String m_url;
    private String m_description;

    private ApplicationAdvocateHome aaHome = null;
    private ApplicationAdvocate aaAdvocate = null;


    public void setApplicationid( String applicationid ){
        m_applicationID = applicationid;
    }

    public void setName ( String name ) {
        m_name = name;
    }

    public void setUrl ( String url) {
        this.m_url = url;
    }

    public void setDescription ( String description) {
        this.m_description = description;
    }


    public int doStartTag() throws JspException {
        try {
            if ( aaHome == null ) {
                aaHome = ( ApplicationAdvocateHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  ApplicationAdvocateHome.class,
                                                                    "ApplicationAdvocate" );

                aaAdvocate = aaHome.create();
            }

            aaAdvocate.editApplication(   m_applicationID,
                                            m_name,
                                            m_url,
                                            m_description );

        }
        catch (Exception ex ) {
            throw new JspTagException( "CreateException: " + ex.getMessage() );
        }

        return SKIP_BODY;
    }

    protected void clearProperties()
    {
        m_applicationID = null;
        m_name  = null;
        m_url = null;
        m_description = null;

        super.clearProperties();
    }
}
