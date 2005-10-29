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
import java.util.ArrayList;
import java.util.Collection;

import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.CoreManager;
import org.sipfoundry.sipxconfig.common.SipUri;
import org.sipfoundry.sipxconfig.phone.PhoneDefaults;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;
import org.sipfoundry.sipxconfig.setting.ConfigFileStorage;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingBeanAdapter;

public class SipxServer extends BeanWithSettings implements Server, AliasProvider {
    private String m_configDirectory;

    private ConfigFileStorage m_storage;

    private String m_domainName;

    private SipxReplicationManager m_sipxReplicationContext;

    private PhoneDefaults m_phoneDefaults;

    private CoreManager m_coreContext;

    protected void decorateSettings() {
        Setting settings = getSettings();
        if (settings == null) {
            return;
        }

        m_storage.decorate(settings);

        // XCF-552 - Domain name on a vanilla installation is `hostname -d`
        // evaluating shell expression is beyond the scope of the UI. Therefore we
        // inject the domain name set on this bean because it's been resolved already.
        // When we support the separation of commserver from general sipxconfig configuration,
        // I suspect domain name will be stored in a separate object and that drives the
        // modification of config.defs and is referenced directly by phoneContext and coreContext
        setSettingValue(ServerSettings.DOMAIN_NAME, m_domainName);
        setSettings(settings);
    }

    public void setDomainName(String domainName) {
        m_domainName = domainName;
    }

    public void setSipxReplicationContext(SipxReplicationManager sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }

    public void setPhoneDefaults(PhoneDefaults phoneDefaults) {
        m_phoneDefaults = phoneDefaults;
    }

    public void setCoreContext(CoreManager coreContext) {
        m_coreContext = coreContext;
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
        String newDomainName = getServerSettings().getDomainName();
        // bail if domain name wasn't changed.
        if (m_domainName.equals(newDomainName)) {
            return;
        }

        m_domainName = newDomainName;

        // unwelcome dependencies, resolve when domain name editing
        // refactored.
        m_phoneDefaults.setDomainName(m_domainName);
        m_coreContext.setDomainName(m_domainName);

        m_sipxReplicationContext.generateAll();
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
        m_storage = new ConfigFileStorage(m_configDirectory);
    }

    public ServerSettings getServerSettings() {
        SettingBeanAdapter adapter = new SettingBeanAdapter(ServerSettings.class);
        adapter.setSetting(getSettings());
        adapter.addMapping("domainName", ServerSettings.DOMAIN_NAME);
        adapter.addMapping("presenceSignInCode", ServerSettings.PRESENCE_SIGN_IN_CODE);
        adapter.addMapping("presenceSignOutCode", ServerSettings.PRESENCE_SIGN_OUT_CODE);
        adapter.addMapping("presenceServerSipPort", ServerSettings.PRESENCE_SERVER_SIP_PORT);

        return (ServerSettings) adapter.getImplementation();
    }

    public Collection getAliasMappings() {
        Collection aliases = new ArrayList();
        String domainName = m_coreContext.getDomainName();
        int presencePort = getPresenceServerPort();
        String signInCode = getSettingValue(ServerSettings.PRESENCE_SIGN_IN_CODE);
        String signOutCode = getSettingValue(ServerSettings.PRESENCE_SIGN_OUT_CODE);
        String presenceServer = getPresenceServerLocation();

        aliases.add(createPresenceAliaseMapping(signInCode.trim(), domainName, presenceServer,
                presencePort));
        aliases.add(createPresenceAliaseMapping(signOutCode.trim(), domainName, presenceServer,
                presencePort));

        return aliases;
    }

    private String getPresenceServerLocation() {
        return getSettingValue(ServerSettings.PRESENCE_SERVER_LOCATION);
    }

    private int getPresenceServerPort() {
        return ((Integer) getSettingTypedValue(ServerSettings.PRESENCE_SERVER_SIP_PORT))
                .intValue();
    }

    private AliasMapping createPresenceAliaseMapping(String code, String domainName,
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
