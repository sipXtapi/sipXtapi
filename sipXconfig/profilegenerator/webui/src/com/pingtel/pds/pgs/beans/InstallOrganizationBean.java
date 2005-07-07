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

import java.io.FileNotFoundException;
import java.rmi.RemoteException;
import java.util.HashMap;
import java.util.Map;
import java.util.Properties;
import java.util.logging.ErrorManager;

import javax.ejb.CreateException;
import javax.naming.NamingException;

import com.pingtel.commserver.utility.GlobalConfigUIHelper;
import com.pingtel.pds.common.ConfigFileManager;
import com.pingtel.pds.common.EJBHomeFactory;
import com.pingtel.pds.common.ErrorMessageBuilder;
import com.pingtel.pds.common.PDSDefinitions;
import com.pingtel.pds.common.PDSException;
import com.pingtel.pds.common.PathLocatorUtil;
import com.pingtel.pds.pgs.organization.OrganizationAdvocate;
import com.pingtel.pds.pgs.organization.OrganizationAdvocateHome;
import com.pingtel.pds.pgs.sipxchange.process.ProcessManager;

/**
 * InstallOrganizationBean
 */
public class InstallOrganizationBean {
    final private String ERR_INSTALL_ORG = "An error occured while trying to install organization";  
 
    
    String m_dnsDomain;

    String m_realm;

    String m_stereotype;

    String m_introText = "";

    String m_orgName = "";

    String m_password = "";

    public InstallOrganizationBean() throws PDSException {
        Properties pgsProps = getPgsProps();
        m_dnsDomain = pgsProps.getProperty(PathLocatorUtil.PGS_SIPXCHANGE_DOMAIN_NAME, "");
        m_realm = pgsProps.getProperty(PathLocatorUtil.PGS_SIPXCHANGE_REALM, "");

        String installStereotype = pgsProps.getProperty("installStereotype",
                PDSDefinitions.ENTERPRISE_ST);
        if (installStereotype.equalsIgnoreCase(PDSDefinitions.ENTERPRISE_ST)) {
            m_introText = "Enterprise Edition Installation";
            m_stereotype = String.valueOf(PDSDefinitions.ORG_ENTERPRISE);
        }
        // service provider to be added in the future...
    }

    public void install() throws PDSException {
        Properties pgsProps = getPgsProps();
        pgsProps.setProperty(PathLocatorUtil.PGS_SIPXCHANGE_REALM, m_realm);

        updateConfigDefs();

        try {
            OrganizationAdvocateHome organizationAdvocateHome = (OrganizationAdvocateHome) EJBHomeFactory
                    .getInstance().getHomeInterface(OrganizationAdvocateHome.class,
                            "OrganizationAdvocate");
            OrganizationAdvocate orgAdvocate = organizationAdvocateHome.create();

            orgAdvocate.install(m_orgName, m_stereotype, m_dnsDomain, m_password);
        } catch (NamingException e) {
            throw new PDSException(ERR_INSTALL_ORG, e);
        } catch (RemoteException e) {
            throw new PDSException(ERR_INSTALL_ORG, e);
        } catch (CreateException e) {
            throw new PDSException(ERR_INSTALL_ORG, e);
        }
    }

    /**
     * Updates the value of the realm in the properties and in the config.defs
     * file Restarts the servers to make sure new config.defs are applied
     * 
     * @param dnsDomain new domain value
     * @param realm new realm value
     */
    private void updateConfigDefs() {
        GlobalConfigUIHelper helper = new GlobalConfigUIHelper();
        Map map = new HashMap();
        map.put(GlobalConfigUIHelper.SIPXCHANGE_DOMAIN_NAME, m_dnsDomain);
        map.put(GlobalConfigUIHelper.SIPXCHANGE_REALM, m_realm);
        helper.replaceAndWrite(map);

        ProcessManager processManager = ProcessManager.getInstance();
        processManager.restartServers();
    }

    private Properties getPgsProps() throws PDSException {
        try {
            PathLocatorUtil plu = PathLocatorUtil.getInstance();
            String path = plu.getPath(PathLocatorUtil.CONFIG_FOLDER, PathLocatorUtil.PGS)
                    + PathLocatorUtil.PGS_PROPS;
            ConfigFileManager cfm = ConfigFileManager.getInstance();
            Properties pgsProps = cfm.getProperties(path);
            return pgsProps;
        } catch (FileNotFoundException e) {
            throw new PDSException(ERR_INSTALL_ORG, e);
        }
    }

    public String getDnsDomain() {
        return m_dnsDomain;
    }

    public void setDnsDomain(String dnsDomain) {
        m_dnsDomain = dnsDomain;
    }

    public String getIntroText() {
        return m_introText;
    }

    public void setIntroText(String introText) {
        m_introText = introText;
    }

    public String getRealm() {
        return m_realm;
    }

    public void setRealm(String realm) {
        m_realm = realm;
    }

    public String getStereotype() {
        return m_stereotype;
    }

    public void setStereotype(String stereotype) {
        m_stereotype = stereotype;
    }

    public String getOrgName() {
        return m_orgName;
    }

    public void setOrgName(String orgName) {
        m_orgName = orgName;
    }

    public String getPassword() {
        return m_password;
    }

    public void setPassword(String password) {
        m_password = password;
    }
}
