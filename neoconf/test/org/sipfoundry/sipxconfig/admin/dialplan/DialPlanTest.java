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
package org.sipfoundry.sipxconfig.admin.dialplan;

import javax.faces.model.SelectItem;

import junit.framework.TestCase;

/**
 * DialPlanTest
 */
public class DialPlanTest extends TestCase {

    public void testGetMapForRange() {
        DialPlan plan = new DialPlan();
        SelectItem[] range = plan.getMapForRange(0,3);
        assertEquals(4,range.length);
        assertEquals(new Integer(1),range[1].getValue());
        SelectItem[] empty = plan.getMapForRange(3,0);
        assertEquals(0,empty.length);
    }

}
