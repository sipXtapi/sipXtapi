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

import java.util.Date;

import org.sipfoundry.sipxconfig.phone.JobRecord;

import junit.framework.TestCase;

public class JobRecordTest extends TestCase {
    
    public void testSetStartDate() {
        JobRecord j = new JobRecord();
        Date expected = new Date();
        j.setStartTime(expected);
        Date actual = j.getStartTime();
        // anything after seconds get truncated, so won't be identical
        assertEquals(expected.toString(), actual.toString());
    }
    
    public void testSetStartStatus() {
        JobRecord j = new JobRecord();
        assertNull(j.getStartTimeString());
        j.setStatus(JobRecord.STATUS_STARTED);
        assertNotNull(j.getStartTimeString());
    }

}
