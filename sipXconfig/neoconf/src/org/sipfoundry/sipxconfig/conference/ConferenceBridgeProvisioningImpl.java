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
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.admin.commserver.SipxReplicationContext;
import org.sipfoundry.sipxconfig.admin.commserver.configdb.ConfigDbParameter;
import org.sipfoundry.sipxconfig.admin.commserver.configdb.ConfigDbSettingAdaptor;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.setting.ProfileNameHandler;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingFilter;
import org.sipfoundry.sipxconfig.setting.SettingUtil;
import org.sipfoundry.sipxconfig.setting.SettingValue2;
import org.sipfoundry.sipxconfig.setting.SettingValueImpl;
import org.springframework.orm.hibernate3.support.HibernateDaoSupport;
import org.sipfoundry.sipxconfig.xmlrpc.XmlRpcClientInterceptor;
import org.apache.xmlrpc.XmlRpcClient;
import org.apache.xmlrpc.XmlRpcClientRequest;
import org.apache.xmlrpc.XmlRpcException;
import org.springframework.aop.framework.ProxyFactory;


public class ConferenceBridgeProvisioningImpl extends HibernateDaoSupport implements
        ConferenceBridgeProvisioning {
    private SipxReplicationContext m_sipxReplicationContext;

    public interface FreeSWITCHFunctions {
        void freeswitch_api(String funcion, String params);
    }
    
    public void deploy(Serializable bridgeId) {
        try {

            Bridge bridge = (Bridge) getHibernateTemplate().load(Bridge.class, bridgeId);
            // get settings for bridge and all conferences
   
            XmlRpcClientInterceptor interceptor = new XmlRpcClientInterceptor();
            interceptor.setServiceInterface(FreeSWITCHFunctions.class);
            interceptor.setServiceUrl(bridge.getServiceUri());
            interceptor.afterPropertiesSet();

            FreeSWITCHFunctions proxy = (FreeSWITCHFunctions) ProxyFactory.getProxy(FreeSWITCHFunctions.class,
                    interceptor);

            proxy.freeswitch_api("reloadxml","");
            generateAdmissionData();
            m_sipxReplicationContext.generate(DataSet.ALIAS);
        } catch (MalformedURLException e) {
            throw new RuntimeException(e);
        }
    }

    void generateAdmissionData() {
        List conferences = getHibernateTemplate().loadAll(Conference.class);
        ConferenceAdmission admission = new ConferenceAdmission();
        admission.generate(conferences);
        m_sipxReplicationContext.replicate(admission);
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
        }
        adaptor.set("bbridge.conf", allSettings);
        getHibernateTemplate().saveOrUpdateAll(conferences);
    }
 	
    public void setSipxReplicationContext(SipxReplicationContext sipxReplicationContext) {
    	m_sipxReplicationContext = sipxReplicationContext;
    }

    public static class BostonBridgeFilter implements SettingFilter {
    	private static final String PREFIX = "fs-conf";
        public boolean acceptSetting(Setting root_, Setting setting) {
            String profileName = setting.getProfileName();
            return profileName.startsWith(PREFIX);
    	}
    }
    	
    public static class ConferenceProfileName implements ProfileNameHandler {
        private static final char SEPARATOR = '.';
        private final String m_conferenceName;

        ConferenceProfileName(String conferenceName) {
            m_conferenceName = SEPARATOR + conferenceName;            
        }
        
        public SettingValue2 getProfileName(Setting setting) {
            String profileName = setting.getProfileName();
            StringBuffer buffer = new StringBuffer(profileName);
            int dotIndex = profileName.indexOf(SEPARATOR);
            if (dotIndex > 0) {
                buffer.insert(dotIndex, m_conferenceName);
            } else {
                buffer.append(m_conferenceName);
            }
            
            return new SettingValueImpl(buffer.toString());                        
        }
    }
}
