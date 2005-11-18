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
package org.sipfoundry.sipxconfig.conference;

import java.io.IOException;
import java.io.Serializable;
import java.net.MalformedURLException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Transformer;
import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.configdb.ConfigDbParameter;
import org.sipfoundry.sipxconfig.admin.commserver.configdb.ConfigDbSettingAdaptor;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDecorator;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingUtil;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcProxyFactoryBean;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class ConferenceBridgeProvisioningImpl extends HibernateDaoSupport implements
        ConferenceBridgeProvisioning {
    private SipxReplicationContext m_sipxReplicationContext;

    private String m_configDirectory;

    public void deploy(Serializable bridgeId) {
        try {
            Bridge bridge = (Bridge) getHibernateTemplate().load(Bridge.class, bridgeId);
            // get settings for bridge and all conferences

            XmlRpcProxyFactoryBean factory = new XmlRpcProxyFactoryBean();
            factory.setServiceInterface(ConfigDbParameter.class);
            factory.setServiceUrl(bridge.getServiceUri());
            factory.afterPropertiesSet();
            ConfigDbParameter configDb = (ConfigDbParameter) factory.getObject();
            ConfigDbSettingAdaptor adaptor = new ConfigDbSettingAdaptor();
            adaptor.setConfigDbParameter(configDb);
            deploy(bridge, adaptor);
            generateAdmissionData();
            m_sipxReplicationContext.generate(DataSet.ALIAS);
        } catch (MalformedURLException e) {
            throw new RuntimeException(e);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    private void generateAdmissionData() throws IOException {
        List conferences = getHibernateTemplate().loadAll(Conference.class);
        ConferenceAdmission admission = new ConferenceAdmission();
        admission.setConfigDirectory(m_configDirectory);
        admission.generate(conferences);
        admission.writeToFile();
    }

    void deploy(Bridge bridge, ConfigDbSettingAdaptor adaptor) {
        // TODO - need to remove deleted conferences
        Collection allSettings = new ArrayList();
        Setting settings = bridge.getSettings();
        SettingFilter bbFilter = new BostonBridgeFilter();
        allSettings.addAll(SettingUtil.filter(bbFilter, settings));
        // collect all settings from and push them to adaptor
        Set conferences = bridge.getConferences();
        for (Iterator i = conferences.iterator(); i.hasNext();) {
            Conference conference = (Conference) i.next();
            conference.generateRemoteAdmitSecret();
            String conferenceName = conference.getName();
            ConferenceNameTransformer transformer = new ConferenceNameTransformer(conferenceName);
            Collection bbSettings = SettingUtil.filter(bbFilter, conference.getSettings());
            CollectionUtils.collect(bbSettings, transformer, allSettings);
        }
        adaptor.set("bbridge.conf", allSettings);
        getHibernateTemplate().saveOrUpdateAll(conferences);
    }

    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }

    public static class BostonBridgeFilter implements SettingFilter {
        private static final String PREFIX = "BOSTON_BRIDGE";

        public boolean acceptSetting(Setting root_, Setting setting) {
            String profileName = setting.getProfileName();
            return profileName.startsWith(PREFIX);
        }
    }

    public static class ConferenceNameDecorator extends SettingDecorator {
        private static final char SEPARATOR = '.';

        private final String m_conferenceName;

        ConferenceNameDecorator(Setting delegate, String conferenceName) {
            super(delegate);
            m_conferenceName = SEPARATOR + conferenceName;
        }

        public String getProfileName() {
            String profileName = getDelegate().getProfileName();
            StringBuffer buffer = new StringBuffer(profileName);
            int dotIndex = profileName.indexOf(SEPARATOR);
            if (dotIndex > 0) {
                buffer.insert(dotIndex, m_conferenceName);
            } else {
                buffer.append(m_conferenceName);
            }
            return buffer.toString();
        }
    }

    public static class ConferenceNameTransformer implements Transformer {
        private String m_conferenceName;

        public ConferenceNameTransformer(String name) {
            m_conferenceName = name;
        }

        public Object transform(Object input) {
            return new ConferenceNameDecorator((Setting) input, m_conferenceName);
        }
    }
}
