/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package com.pingtel.pds.pgs.beans;

import java.rmi.RemoteException;
import java.util.Collection;
import java.util.Iterator;

import javax.ejb.CreateException;
import javax.ejb.FinderException;
import javax.naming.NamingException;

import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.pgs.organization.Organization;
import com.pingtel.pds.pgs.organization.OrganizationAdvocate;
import com.pingtel.pds.pgs.organization.OrganizationAdvocateHome;
import com.pingtel.pds.pgs.organization.OrganizationHome;

/**
 * EditOrganizationBean - support bean for edit_organization.jsp page.
 * Used as a session scope bean.
 */
public class EditOrganizationBean {
    // TODO: need to externalize - however message builder is not easily accessible here 
    private static final String ERROR_EDIT_MSG = "Error editing organization settings.";
    private static final String ERROR_ACCESS_MSG = "Error accessing organization settings.";
    private static final String ERROR_MANY_ORG = "Error: multiple organizations defined. Cannot change organization settings";
    
    private String m_name;

    private String m_dnsDomain;

    private String m_orgID;

    public EditOrganizationBean() throws PDSException {
        try {
            final EJBHomeFactory factory = EJBHomeFactory.getInstance();
            OrganizationHome orgHome = (OrganizationHome) factory.getHomeInterface(
                    OrganizationHome.class, "Organization");
            Collection orgs = orgHome.findAll();
            if (orgs.size() != 1) {
                throw new PDSException(ERROR_MANY_ORG);
            }

            Iterator i = orgs.iterator();
            Organization org = (Organization) i.next();
            m_name = org.getName();
            m_dnsDomain = org.getDNSDomain();

            m_orgID = org.getID().toString();            
        } catch (NamingException e) {
            throw new PDSException(ERROR_ACCESS_MSG, e);
        } catch (RemoteException e) {
            throw new PDSException(ERROR_ACCESS_MSG, e);
        } catch (FinderException e) {
            throw new PDSException(ERROR_ACCESS_MSG, e);
        }
    }

    public void submit() throws PDSException {
        try {
            final EJBHomeFactory factory = EJBHomeFactory.getInstance();
            OrganizationAdvocateHome orgAdvHome = (OrganizationAdvocateHome) factory
                    .getHomeInterface(OrganizationAdvocateHome.class, "OrganizationAdvocate");
            OrganizationAdvocate oa = orgAdvHome.create();
            oa.editOrganization(m_orgID, m_name, null, m_dnsDomain);
        } catch (NamingException e) {
            throw new PDSException(ERROR_EDIT_MSG, e);
        } catch (RemoteException e) {
            throw new PDSException(ERROR_EDIT_MSG, e);
        } catch (CreateException e) {
            throw new PDSException(ERROR_EDIT_MSG, e);
        }
    }

    public String getDnsDomain() {
        return m_dnsDomain;
    }

    public void setDnsDomain(String dnsDomain) {
        m_dnsDomain = dnsDomain.trim();
    }

    public String getName() {
        return m_name;
    }

    public void setName(String name) {
        m_name = name.trim();
    }
}
