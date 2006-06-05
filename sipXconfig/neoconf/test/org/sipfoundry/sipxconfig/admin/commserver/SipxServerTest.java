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

import java.io.InputStream;
import java.util.Collection;
import java.util.Iterator;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.DataSet;
import org.sipfoundry.sipxconfig.admin.forwarding.AliasMapping;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxServerTest extends TestCase {

    private SipxServer m_server;

    protected void setUp() throws Exception {
        m_server = new SipxServer();
        m_server.setConfigDirectory(TestHelper.getTestDirectory());
        InputStream configDefs = getClass().getResourceAsStream("config.defs");
        TestHelper
                .copyStreamToDirectory(configDefs, TestHelper.getTestDirectory(), "config.defs");
        InputStream sipxpresence = getClass().getResourceAsStream("sipxpresence-config.test.in");
        TestHelper.copyStreamToDirectory(sipxpresence, TestHelper.getTestDirectory(),
                "sipxpresence-config.in");
        // we read server location from sipxpresence-config
        sipxpresence = getClass().getResourceAsStream("sipxpresence-config.test.in");
        TestHelper.copyStreamToDirectory(sipxpresence, TestHelper.getTestDirectory(),
                "sipxpresence-config");
        m_server.setModelFilesContext(TestHelper.getModelFilesContext());
    }

    public void testGetSetting() {
        Setting settings = m_server.getSettings();
        assertNotNull(settings);
    }

    public void testDomainNameChange() {
        String newDomainName = "new-domain-name";

        IMocksControl phoneDefaultsCtrl = org.easymock.classextension.EasyMock.createControl();
        DeviceDefaults deviceDefaults = phoneDefaultsCtrl.createMock(DeviceDefaults.class);
        deviceDefaults.setDomainName(newDomainName);
        phoneDefaultsCtrl.replay();

        IMocksControl coreContextCtrl = EasyMock.createControl();
        CoreContext coreContext = (CoreContext) coreContextCtrl.createMock(CoreContext.class);
        coreContext.getDomainName();
        coreContextCtrl.andReturn("old-domain-name").anyTimes();
        coreContext.setDomainName(newDomainName);
        coreContextCtrl.replay();

        IMocksControl replicationContextCtrl = EasyMock.createControl();
        SipxReplicationContext replicationContext = replicationContextCtrl.createMock(
                SipxReplicationContext.class);
        replicationContext.generate(DataSet.ALIAS);
        replicationContext.generateAll();
        replicationContextCtrl.replay();

        m_server.setPhoneDefaults(deviceDefaults);
        m_server.setCoreContext(coreContext);
        m_server.setSipxReplicationContext(replicationContext);

        m_server.setSettingValue("domain/SIPXCHANGE_DOMAIN_NAME", newDomainName);
        m_server.applySettings();

        replicationContextCtrl.verify();
        phoneDefaultsCtrl.verify();
        coreContextCtrl.verify();
    }
    
    public void testGetAliasMappings() {
        IMocksControl coreContextCtrl = EasyMock.createControl();
        CoreContext coreContext = coreContextCtrl.createMock(CoreContext.class);
        coreContext.getDomainName();
        coreContextCtrl.andReturn("domain.com").atLeastOnce(); 
        coreContextCtrl.replay();

        m_server.setCoreContext(coreContext);
        assertNotNull(m_server.getPresenceServerUri());
        
        Collection aliasMappings = m_server.getAliasMappings();

        assertEquals(2, aliasMappings.size());
        for (Iterator i = aliasMappings.iterator(); i.hasNext();) {
            AliasMapping am = (AliasMapping) i.next();
            assertTrue(am.getIdentity().matches("\\*8\\d@domain.com"));
            assertTrue(am.getContact().matches("sip:\\*8\\d@presence.server.com:\\d+"));
        }

        coreContextCtrl.verify();
    }

    public void testGetPresenceServerUri() {
        assertEquals("sip:presence.server.com:5130", m_server.getPresenceServerUri());
    }
}
