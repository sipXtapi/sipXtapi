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

import junit.framework.TestCase;

import org.dbunit.Assertion;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ITable;
import org.dbunit.dataset.ReplacementDataSet;
import org.easymock.MockControl;
import org.easymock.classextension.MockClassControl;
import org.sipfoundry.sipxconfig.TestHelper;

public class JobRecordTestDb extends TestCase {

    private PhoneContext m_phoneContext;
    
    protected void setUp() throws Exception {
        m_phoneContext = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
    }
    
    public void testStoreJob() throws Exception {
        TestHelper.cleanInsert("ClearDb.xml");
        
        JobRecord job = new JobRecord();
        job.setStatus(JobRecord.STATUS_STARTED);
        job.setDetails("store details");
        job.setExceptionMessage("store exception message");
        job.setProgress("store progress");
        job.setStartTimeString("store start time");
        job.setType(JobRecord.TYPE_DEVICE_RESTART);
        m_phoneContext.storeJob(job);
        
        IDataSet expectedDs = TestHelper.loadDataSetFlat("phone/SaveJobRecordExpected.xml"); 
        ReplacementDataSet expectedRds = new ReplacementDataSet(expectedDs);
        expectedRds.addReplacementObject("[job_id]", job.getId());        
        
        ITable expected = expectedRds.getTable("job");                
        ITable actual = TestHelper.getConnection().createDataSet().getTable("job");
        
        Assertion.assertEquals(expected, actual);                
    }
    

    public void testLoadJob() throws Exception {
        TestHelper.cleanInsertFlat("phone/JobRecordSeed.xml");
        
        JobRecord job = m_phoneContext.loadJob(new Integer(1000));
        assertEquals("load details", job.getDetails());
        assertEquals("load exception message", job.getExceptionMessage());
        assertEquals("load progress", job.getProgress());
        assertEquals("load start date", job.getStartTimeString());
        assertEquals(JobRecord.TYPE_DEVICE_RESTART, job.getType());
        assertEquals(JobRecord.STATUS_STARTED, job.getStatus());        
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
