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
    
    private Class m_class = EndpointTestDb.class;
        
    protected void setUp() throws Exception {
        m_context = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);        
    }
    
    public void testLoad() throws Exception {        
        TestHelper.cleanInsert(m_class, "ClearDb.xml");
        TestHelper.cleanInsertFlat(m_class, "PhoneSummarySeed.xml");
        
        List summaries = m_context.loadPhoneSummaries();
        
        assertEquals(3, summaries.size());
    }
}
