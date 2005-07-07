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

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;


public class PhoneContextTestDb extends TestCase {

    private PhoneContext m_context;
    
    protected void setUp() throws Exception {
        m_context = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
    }
        
    public void testClear() {
        //todo, seed some data
        m_context.clear();
    }
}
