/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.commserver.imdb;

import java.util.Iterator;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

public class DataSetGeneratorTest extends TestCase {
    public void testDataSetGeneratorBeans() {
        ApplicationContext ac = TestHelper.getApplicationContext();
        for (Iterator i = DataSet.iterator(); i.hasNext();) {
            DataSet set = (DataSet) i.next();
            DataSetGenerator gen = (DataSetGenerator) ac.getBean(set.getBeanName(),
                    DataSetGenerator.class);
            assertNotNull(set.getName(), gen);
            assertNotNull(set.getName(), gen.getCoreContext());
        }
    }
}
