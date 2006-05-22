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

import org.easymock.MockControl;
import org.easymock.classextension.MockClassControl;
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

        MockControl phoneDefaultsCtrl = MockClassControl.createControl(DeviceDefaults.class);
        DeviceDefaults deviceDefaults = (DeviceDefaults) phoneDefaultsCtrl.getMock();
        deviceDefaults.setDomainName(newDomainName);
        phoneDefaultsCtrl.replay();

        MockControl coreContextCtrl = MockControl.createControl(CoreContext.class);
        CoreContext coreContext = (CoreContext) coreContextCtrl.getMock();
        coreContextCtrl.expectAndReturn(coreContext.getDomainName(), "old-domain-name", 
                MockControl.ONE_OR_MORE);
        coreContext.setDomainName(newDomainName);
        coreContextCtrl.replay();

        MockControl replicationContextCtrl = MockControl
                .createControl(SipxReplicationContext.class);
        SipxReplicationContext replicationContext = (SipxReplicationContext) replicationContextCtrl
                .getMock();
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
        MockControl coreContextCtrl = MockControl.createControl(CoreContext.class);
        CoreContext coreContext = (CoreContext) coreContextCtrl.getMock();
        coreContextCtrl.expectAndReturn(coreContext.getDomainName(), "domain.com", 
                MockControl.ONE_OR_MORE);
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
