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
import com.pingtel.pds.pgs.user.ApplicationGroupAdvocate;
import com.pingtel.pds.pgs.user.ApplicationGroupAdvocateHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;


public class DeleteApplicationSetTag extends ExTagSupport {

    private String m_applicationSetID;

    private ApplicationGroupAdvocateHome aaHome = null;
    private ApplicationGroupAdvocate aaAdvocate = null;


    public void setApplicationsetid ( String applicationsetid ){
        m_applicationSetID = applicationsetid;
    }


    public int doStartTag() throws JspException {
        try {
            if ( aaHome == null ) {
                aaHome = ( ApplicationGroupAdvocateHome )
                                    EJBHomeFactory.getInstance().getHomeInterface(  ApplicationGroupAdvocateHome.class,
                                                                                    "ApplicationGroupAdvocate" );

                aaAdvocate = aaHome.create();
            }

            aaAdvocate.deleteApplicationGroup( m_applicationSetID );

        }
        catch (Exception ex ) {
            throw new JspTagException( ex.getMessage() );
        }

        return SKIP_BODY;
    }

    protected void clearProperties() {
        m_applicationSetID = null;

        super.clearProperties();
    }
}
