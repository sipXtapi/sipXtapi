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
import org.sipfoundry.sipxconfig.phone.PhoneDefaults;
import org.sipfoundry.sipxconfig.setting.Setting;

public class SipxServerTest extends TestCase {

    private SipxServer m_server;

    protected void setUp() throws Exception {
        m_server = new SipxServer();
        m_server.setConfigDirectory(TestHelper.getTestDirectory());
        InputStream configDefs = getClass().getResourceAsStream("config.defs");
        TestHelper.copyStreamToDirectory(configDefs, TestHelper.getTestDirectory(), "config.defs");
        InputStream sipxpresence = getClass().getResourceAsStream("sipxpresence-config.test.in");
        TestHelper.copyStreamToDirectory(sipxpresence, TestHelper.getTestDirectory(), "sipxpresence-config.in");        
        m_server.setSettingModel(TestHelper.loadSettings("commserver/server.xml"));
    }    
    
    public void testGetSetting() {
        Setting settings = m_server.getSettings();
        assertNotNull(settings);
    }
    
    public void testGetServerSettings() {
        String expected = "myserver";        
        m_server.getSettings().getSetting("domain/SIPXCHANGE_DOMAIN_NAME").setValue(expected);        
        String actual = m_server.getServerSettings().getDomainName();
        // although not intuative, a new copy of server settings is generated each time.        
        assertEquals(expected, actual);
    }
    
    public void testDomainNameChange() {
        String newDomainName = "new-domain-name";

        MockControl phoneDefaultsCtrl = MockClassControl.createControl(PhoneDefaults.class);
        PhoneDefaults phoneDefaults = (PhoneDefaults) phoneDefaultsCtrl.getMock();
        phoneDefaults.setDomainName(newDomainName);
        phoneDefaultsCtrl.replay();
        
        MockControl coreContextCtrl = MockControl.createControl(CoreContext.class);
        CoreContext coreContext = (CoreContext) coreContextCtrl.getMock();
        coreContext.setDomainName(newDomainName);
        coreContextCtrl.replay();
        
        MockControl replicationContextCtrl = MockControl.createControl(SipxReplicationContext.class);
        SipxReplicationContext replicationContext = (SipxReplicationContext) replicationContextCtrl.getMock();
        replicationContext.generate(DataSet.ALIAS);
        replicationContext.generateAll();
        replicationContextCtrl.replay();

        m_server.setPhoneDefaults(phoneDefaults);
        m_server.setCoreContext(coreContext);
        m_server.setSipxReplicationContext(replicationContext);
        
        m_server.setDomainName("old-domain-name");
        m_server.getSettings().getSetting("domain/SIPXCHANGE_DOMAIN_NAME").setValue(newDomainName);
        m_server.applySettings();        

        replicationContextCtrl.verify();
        phoneDefaultsCtrl.verify();
        coreContextCtrl.verify();
    }
    
    public void testGetAliasMappings() {
        MockControl phoneDefaultsCtrl = MockClassControl.createControl(PhoneDefaults.class);
        PhoneDefaults phoneDefaults = (PhoneDefaults) phoneDefaultsCtrl.getMock();
        phoneDefaults.getOutboundProxy();
        phoneDefaultsCtrl.setDefaultReturnValue("presence.com");
        phoneDefaultsCtrl.replay();
        
        MockControl coreContextCtrl = MockControl.createControl(CoreContext.class);
        CoreContext coreContext = (CoreContext) coreContextCtrl.getMock();
        coreContext.getDomainName();
        coreContextCtrl.setDefaultReturnValue("domain.com");        
        coreContextCtrl.replay();        
        
        m_server.setPhoneDefaults(phoneDefaults);
        m_server.setCoreContext(coreContext);        
        Collection aliasMappings = m_server.getAliasMappings();
        
        assertEquals(2, aliasMappings.size());
        for (Iterator i = aliasMappings.iterator(); i.hasNext();) {
            AliasMapping am = (AliasMapping) i.next();
            assertTrue(am.getIdentity().matches("\\*8\\d@domain.com"));
            assertTrue(am.getContact().matches("sip:\\*8\\d@presence.com:\\d+"));            
        }
        
        phoneDefaultsCtrl.verify();
        coreContextCtrl.verify();
    }
}
