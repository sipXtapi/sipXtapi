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

import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;
import org.springframework.context.support.ClassPathXmlApplicationContext;

/**
 * Comments
 */
public final class PhoneTestHelper {       
    
    public static UnitTestDao getUnitTestDao() {
        String[] locations = new String[] {
                UnitTestDao.APPLICATION_CONTEXT_FILE
        };
        ApplicationContext unittest = new ClassPathXmlApplicationContext(locations, 
                true, TestHelper.getApplicationContext());        

        return (UnitTestDao) unittest.getBean("testData");
    }
    
}