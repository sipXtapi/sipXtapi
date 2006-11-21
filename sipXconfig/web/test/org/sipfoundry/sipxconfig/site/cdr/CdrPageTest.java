/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.cdr;

import java.util.Calendar;
import java.util.Date;

import junit.framework.TestCase;

public class CdrPageTest extends TestCase {

    public void testPageBeginRender() {
        Calendar now = Calendar.getInstance();
        Date defaultEndTime = CdrPage.getDefaultEndTime();
        Calendar endTime = Calendar.getInstance();
        endTime.setTime(defaultEndTime);
        assertTrue(endTime.after(now));
        assertEquals(0, endTime.get(Calendar.HOUR_OF_DAY));
        assertEquals(0, endTime.get(Calendar.MINUTE));
        assertEquals(0, endTime.get(Calendar.SECOND));
    }

}
