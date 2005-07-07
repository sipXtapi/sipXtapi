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
package org.sipfoundry.sipxconfig.common;

import java.util.ArrayList;
import java.util.List;

import junit.framework.TestCase;

public class DaoUtilsTest extends TestCase {
    
    BeanWithId subject = new BeanWithId();
        
    List objs = new ArrayList();

    protected void setUp() {
        subject.setUniqueId();
    }

    public void testCheckDuplicatesFoundItself() {
        objs.add(new Integer(subject.getId().intValue()));
        DaoUtils.checkDuplicates(subject, objs, new UserException());
    }
    
    public void testCheckDuplicatesFoundDuplicate() {
        objs.add(new Integer(subject.getId().intValue() + 1));
        try {
            DaoUtils.checkDuplicates(subject, objs, new UserException());
            fail();
        } catch (UserException expected) {
            assertTrue(true);
        }        
    }
}
