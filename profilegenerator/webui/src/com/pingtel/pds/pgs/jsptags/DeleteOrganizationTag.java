/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/DeleteOrganizationTag.java#4 $
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
import com.pingtel.pds.pgs.organization.OrganizationAdvocate;
import com.pingtel.pds.pgs.organization.OrganizationAdvocateHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;

public class DeleteOrganizationTag extends ExTagSupport {

    private String m_orgID = null;
    private OrganizationAdvocateHome oaHome = null;
    private OrganizationAdvocate orgAdvocate = null;

    public void setOrgid( String orgid ) {
        m_orgID = orgid;
    }

    public int doStartTag() throws JspException {
        try {
            if ( oaHome == null ) {
                oaHome = ( OrganizationAdvocateHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  OrganizationAdvocateHome.class,
                                                                    "OrganizationAdvocate" );
                orgAdvocate = oaHome.create();
            }

            orgAdvocate.deleteOrganization( m_orgID );
        }
        catch (Exception ex ) {
            throw new JspTagException( ex.getMessage() );
        }

        return SKIP_BODY;
    }


    protected void clearProperties() {
        m_orgID = null;

        super.clearProperties();
    }
}