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
        
        List summaries = m_context.loadPhoneSummaries();
        
        assertEquals(3, summaries.size());
        PhoneSummary[] summariesArray = (PhoneSummary[]) summaries.toArray(new PhoneSummary[0]);
        assertEquals(1, summariesArray[0].getLines().size());
        assertEquals(0, summariesArray[1].getLines().size());
        assertEquals(2, summariesArray[2].getLines().size());
    }
}
