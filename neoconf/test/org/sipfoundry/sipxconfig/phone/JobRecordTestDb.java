/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import java.util.ArrayList;
import java.util.List;

import org.easymock.MockControl;
import org.easymock.classextension.MockClassControl;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;

public class JobRecordTestDb extends SipxDatabaseTestCase {

    private PhoneContext m_phoneContext;
    
    protected void setUp() throws Exception {
        m_phoneContext = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
    }
        
    public void testRunJob() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");

        MockControl phoneControl = MockClassControl.createStrictControl(Phone.class);
        Phone phone = (Phone) phoneControl.getMock();
        phoneControl.expectAndReturn(phone.getSerialNumber(), "000000000000");
        phone.restart();
        phoneControl.setVoidCallable(1);
        phoneControl.replay();
        
        List phones = new ArrayList();
        phones.add(phone);
        
        m_phoneContext.restart(phones);
        JobManager jobMgr = (JobManager) TestHelper.getApplicationContext().getBean("jobManager");
        jobMgr.finishProcessingJobs();
        
        phoneControl.verify();
    }
}
