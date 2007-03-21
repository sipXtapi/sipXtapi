/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.service;

import java.util.Collection;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.device.ModelSource;

public class ServiceManagerTestDb extends TestHelper.TestCaseDb {
    private ServiceManager m_manager;
    
    protected void setUp() {
        m_manager = (ServiceManager) TestHelper.getApplicationContext().getBean(ServiceManager.CONTEXT_ID);        
    }
    
    public void testModelSource() {
        ModelSource source = (ModelSource) TestHelper.getApplicationContext().getBean("serviceDescriptorSource");
        assertNotNull(source.getModel("ntpService"));        
    }

    public void testNewService() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        ConfiguredService ntp = m_manager.newService(UnmanagedService.NTP);
        assertNotNull(ntp);
        ntp.setName("ntp service");
        ntp.setAddress("ntp.pools.org");
        m_manager.saveService(ntp);
    }
    
    public void testLoadService() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("service/ServiceSeed.db.xml");
        ConfiguredService ntp = m_manager.loadService(1000);
        assertNotNull(ntp);        
        Collection<ConfiguredService> services = m_manager.getServicesByType(UnmanagedService.NTP);
        assertEquals(1, services.size());
        assertSame(UnmanagedService.NTP, services.iterator().next().getDescriptor());
    }
    
    public void testCheckDuplicateName() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        TestHelper.cleanInsertFlat("service/ServiceSeed.db.xml");
        ConfiguredService ntp = m_manager.newService(UnmanagedService.NTP);
        ntp.setName("my-ntp");
        try {
            m_manager.saveService(ntp);
            fail();
        } catch (UserException expected) {
            assertTrue(true);
        }        
    }
}
