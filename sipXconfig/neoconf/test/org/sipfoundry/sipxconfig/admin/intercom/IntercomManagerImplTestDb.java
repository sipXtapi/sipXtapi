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
package org.sipfoundry.sipxconfig.admin.intercom;

import java.util.List;

import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.admin.intercom.Intercom;
import org.sipfoundry.sipxconfig.admin.intercom.IntercomManagerImpl;
import org.springframework.context.ApplicationContext;

public class IntercomManagerImplTestDb extends SipxDatabaseTestCase {
    private IntercomManagerImpl m_intercomManager;

    protected void setUp() throws Exception {
        ApplicationContext app = TestHelper.getApplicationContext();
        m_intercomManager = (IntercomManagerImpl) app.getBean(IntercomManagerImpl.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
    }
    
    public void testNewIntercom() {
        Intercom intercom = m_intercomManager.newIntercom();
        assertNotNull(intercom);
    }

    public void testSaveIntercom() {
        // create and save an intercom
        Intercom intercom = m_intercomManager.newIntercom();
        final String PREFIX = "77";
        intercom.setPrefix(PREFIX);
        final int TIMEOUT = 123;
        intercom.setTimeout(TIMEOUT);
        final String CODE = "whatever";
        intercom.setCode(CODE);
        m_intercomManager.saveIntercom(intercom);
        intercom = null;    // be really sure we're not holding on to it
        
        // load it back up and check it
        List intercoms = m_intercomManager.loadIntercoms();
        assertEquals(1, intercoms.size());
        intercom = (Intercom) intercoms.get(0);
        assertEquals(PREFIX, intercom.getPrefix());
        assertEquals(TIMEOUT, intercom.getTimeout());
        assertEquals(CODE, intercom.getCode());
    }

    public void testLoadIntercoms() throws Exception {
        // verify loading the sample data
        TestHelper.insertFlat("admin/intercom/SampleIntercoms.xml");
        List intercoms = m_intercomManager.loadIntercoms();
        assertEquals(2, intercoms.size());
        Intercom i1 = (Intercom) intercoms.get(0);
        assertEquals("66", i1.getPrefix());
        Intercom i2 = (Intercom) intercoms.get(1);
        assertEquals(4000, i2.getTimeout());        
    }

    public void testClear() throws Exception {
        // load some sample intercoms
        TestHelper.insertFlat("admin/intercom/SampleIntercoms.xml");
        
        // blow them all away
        m_intercomManager.clear();
        
        // they should be gone
        List intercoms = m_intercomManager.loadIntercoms();
        assertEquals(0, intercoms.size());
    }
}
