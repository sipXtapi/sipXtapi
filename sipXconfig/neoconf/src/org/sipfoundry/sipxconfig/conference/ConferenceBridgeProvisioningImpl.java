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

import java.io.Serializable;
import java.net.MalformedURLException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.Set;

import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.configdb.ConfigDbParameter;
import org.sipfoundry.sipxconfig.admin.commserver.configdb.ConfigDbSettingAdaptor;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingUtil;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcProxyFactoryBean;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;

public class ConferenceBridgeProvisioningImpl extends HibernateDaoSupport implements
        ConferenceBridgeProvisioning {
    private SipxReplicationContext m_sipxReplicationContext;

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
            m_sipxReplicationContext.generate(DataSet.ALIAS);
        } catch (MalformedURLException e) {
            throw new RuntimeException(e);
        }
    }

    void deploy(Bridge bridge, ConfigDbSettingAdaptor adaptor) {
        // TODO - need to remove deleted conferences
        // TODO - need to filter settings to be set on conference bridge
        Collection allSettings = new ArrayList();
        Setting settings = bridge.getSettings();
        allSettings.addAll(SettingUtil.filter(SettingFilter.ALL, settings));
        // collect all settings from and push them to adaptor
        Set conferences = bridge.getConferences();
        for (Iterator i = conferences.iterator(); i.hasNext();) {
            Conference conference = (Conference) i.next();
            settings = conference.getSettings();
            allSettings.addAll(SettingUtil.filter(SettingFilter.ALL, settings));
        }
        adaptor.set("bbridge.conf", allSettings);
    }

    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
        m_sipxReplicationContext = sipxReplicationContext;
    }
}
