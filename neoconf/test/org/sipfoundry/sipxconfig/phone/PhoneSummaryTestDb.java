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

import java.util.List;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;


public class PhoneSummaryTestDb extends TestCase {

    private PhoneContext m_context;
    
    protected void setUp() throws Exception {
        m_context = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);        
    }
    
    public void testLoad() throws Exception {        
        TestHelper.cleanInsert("dbdata/ClearDb.xml");
        TestHelper.cleanInsertFlat("phone/dbdata/PhoneSummarySeed.xml");
        
        List summaries = m_context.loadPhones();
        
        assertEquals(3, summaries.size());
        Phone[] summariesArray = (Phone[]) summaries.toArray(new Phone[0]);

        assertEquals("unittest-sample phone1", summariesArray[0].getPhoneMetaData().getDisplayLabel());
        assertEquals(1, summariesArray[0].getLineCount());

        assertEquals("unittest-sample phone2", summariesArray[1].getPhoneMetaData().getDisplayLabel());
        assertEquals(0, summariesArray[1].getLineCount());

        assertEquals("unittest-sample phone3", summariesArray[2].getPhoneMetaData().getDisplayLabel());
        assertEquals(2, summariesArray[2].getLineCount());
    }
}
