/*
 * $Id: //depot/OPENDEV/sipXconfig/profilegenerator/webui/src/com/pingtel/pds/pgs/jsptags/EditOrganizationTag.java#4 $
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
import com.pingtel.pds.pgs.organization.Organization;
import com.pingtel.pds.pgs.organization.OrganizationAdvocate;
import com.pingtel.pds.pgs.organization.OrganizationAdvocateHome;
import com.pingtel.pds.pgs.organization.OrganizationHome;

import javax.servlet.jsp.JspException;
import javax.servlet.jsp.JspTagException;


public class EditOrganizationTag extends ExTagSupport {

    private OrganizationHome oHome = null;
    private OrganizationAdvocateHome oaHome = null;
    private String m_orgID;
    private String m_orgName;
    private String m_dnsDomain;

    /**
     * Setter for the organization ID - this is distinct from the session one
     * as we are editing an organization in our hierarchy
     *
     * @param orgid
     */
    public void setOrgid( String orgid ) {
        m_orgID = orgid;
    }

    /**
     * Setter for the organization name
     * @param orgname
     */
    public void setOrgname( String orgname ) {
        m_orgName = orgname;
    }

    /**
     * Setter for the organization ID - this is distinct from the session one
     * as we are editing an organization in our hierarchy
     *
     * @param orgid
     */
    public void setDnsdomain( String dnsdomain ) {
        m_dnsDomain = dnsdomain;
    }

    /**
     *
     * @return
     * @exception JspException
     */
    public int doStartTag() throws JspException {
        try {
            String parentID = null;

            if ( oHome == null ) {
                oHome = ( OrganizationHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  OrganizationHome.class,
                                                                    "Organization" );

                oaHome = ( OrganizationAdvocateHome )
                    EJBHomeFactory.getInstance().getHomeInterface(  OrganizationAdvocateHome.class,
                                                                    "OrganizationAdvocate" );
            }

            OrganizationAdvocate orgAdvocate = oaHome.create();

            Organization organization = oHome.findByPrimaryKey( new Integer(m_orgID) );
            parentID = organization.getParentID().toString();

            orgAdvocate.editOrganization(m_orgID, m_orgName, parentID, m_dnsDomain);
        }
        catch (Exception ex ) {
            throw new JspTagException(  ex.getMessage());
        }

        return SKIP_BODY;
    }


    protected void clearProperties() {
        m_orgID = null;
        m_orgName = null;
        m_dnsDomain = null;

        super.clearProperties();
    }
}