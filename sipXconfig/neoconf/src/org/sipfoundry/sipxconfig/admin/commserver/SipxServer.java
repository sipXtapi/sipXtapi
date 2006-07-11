/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.IOException;
import java.text.MessageFormat;
import java.util.ArrayList;
import java.util.Collection;

import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.ConfigFileStorage;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingEntry;

public class SipxServer extends BeanWithSettings implements Server, AliasProvider {
    private static final String DOMAIN_NAME = "domain/SIPXCHANGE_DOMAIN_NAME";
    private static final String PRESENCE_SIGN_IN_CODE = "presence/SIP_PRESENCE_SIGN_IN_CODE";
    private static final String PRESENCE_SIGN_OUT_CODE = "presence/SIP_PRESENCE_SIGN_OUT_CODE";
    private static final String PRESENCE_SERVER_SIP_PORT = "presence/PRESENCE_SERVER_SIP_PORT";
    // note: the name of the setting is misleading - this is actually full host name not just a
    // domain name
    private static final String PRESENCE_SERVER_LOCATION = "presence/SIP_PRESENCE_DOMAIN_NAME";   
    private static final String PRESENCE_API_PORT = "presence/SIP_PRESENCE_HTTP_PORT";

    private String m_configDirectory;
    private ConfigFileStorage m_storage;
    private SipxReplicationContext m_sipxReplicationContext;
    private DeviceDefaults m_deviceDefaults;
    private CoreContext m_coreContext;

    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }

    public void setPhoneDefaults(DeviceDefaults deviceDefaults) {
        m_deviceDefaults = deviceDefaults;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
    
    @Override
    public void initialize() {
        addDefaultBeanSettingHandler(new SipxDefaults(m_coreContext));
    }
    
    public static class SipxDefaults {
        private CoreContext m_core;
        SipxDefaults(CoreContext core) {
            m_core = core;
        }

        /*
         * XCF-552 - Domain name on a vanilla installation is `hostname -d` evaluating shell
         * expression is beyond the scope of the UI. Therefore we inject the domain name set on
         * this bean because it's been resolved already. When we support the separation of
         * commserver from general sipxconfig configuration, I suspect domain name will be stored
         * in a separate object and that drives the modification of config.defs and is referenced
         * directly by phoneContext and coreContext
         */
        @SettingEntry(path = DOMAIN_NAME)
        public String getDomainName() {
            return m_core.getDomainName();
        }        
    }
    
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("server.xml", "commserver");
    }

    public void applySettings() {
        try {
            handlePossibleDomainNameChange();
            handlePossiblePresenceServerChange();
            m_storage.flush();
        } catch (IOException e) {
            // TODO: catch and report as User Exception
            throw new RuntimeException(e);
        }
    }

    private void handlePossiblePresenceServerChange() {
        // TODO: in reality only need to do that if sing-in/sign-out code changed
        m_sipxReplicationContext.generate(DataSet.ALIAS);
    }

    void handlePossibleDomainNameChange() {
        String newDomainName = getSettingValue(DOMAIN_NAME);
        // bail if domain name wasn't changed.
        if (m_coreContext.getDomainName().equals(newDomainName)) {
            return;
        }

        // unwelcome dependencies, resolve when domain name editing
        // refactored.
        m_deviceDefaults.setDomainName(newDomainName);
        m_coreContext.setDomainName(newDomainName);

        m_sipxReplicationContext.generateAll();
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
        m_storage = new ConfigFileStorage(m_configDirectory);
        setValueStorage(m_storage);
    }

    public Collection getAliasMappings() {
        Collection aliases = new ArrayList();
        String domainName = m_coreContext.getDomainName();
        int presencePort = getPresenceServerPort();
        String signInCode = getSettingValue(PRESENCE_SIGN_IN_CODE);
        String signOutCode = getSettingValue(PRESENCE_SIGN_OUT_CODE);
        String presenceServer = getPresenceServerLocation();

        aliases.add(createPresenceAliasMapping(signInCode.trim(), domainName, presenceServer,
                presencePort));
        aliases.add(createPresenceAliasMapping(signOutCode.trim(), domainName, presenceServer,
                presencePort));

        return aliases;
    }

    public String getPresenceServiceUri() {
        Object[] params = new Object[] {
            getPresenceServerLocation(), 
            String.valueOf(getPresenceServerApiPort())
        };
        return MessageFormat.format("http://{0}:{1}/RPC2", params);
    }

    private String getPresenceServerLocation() {
        return getSettingValue(PRESENCE_SERVER_LOCATION);
    }

    private int getPresenceServerApiPort() {
        return ((Integer) getSettingTypedValue(PRESENCE_API_PORT))
                .intValue();
    }

    private int getPresenceServerPort() {
        return ((Integer) getSettingTypedValue(PRESENCE_SERVER_SIP_PORT))
                .intValue();
    }

    private AliasMapping createPresenceAliasMapping(String code, String domainName,
            String presenceServer, int port) {
        AliasMapping mapping = new AliasMapping();
        mapping.setIdentity(AliasMapping.createUri(code, domainName));
        mapping.setContact(SipUri.format(code, presenceServer, port));
        return mapping;
    }

    public String getPresenceServerUri() {
        return SipUri.format(getPresenceServerLocation(), getPresenceServerPort());
    }
}
