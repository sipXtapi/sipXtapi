/*
 * 
 * 
 * Copyright (C) 2007 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2007 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.site.setting;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.site.setting.SettingsGrid.IndexIterator;

public class SettingsGridTest extends TestCase {

    public void testIterator() {
        IndexIterator i = new SettingsGrid.IndexIterator(5);
        StringBuilder buf = new StringBuilder();
        while (i.hasNext()) {
            buf.append(i.next());            
        }
        assertEquals("01234", buf.toString());        
    }

    public void testEmptyIterator() {
        IndexIterator i = new SettingsGrid.IndexIterator(0);
        StringBuilder buf = new StringBuilder();
        while (i.hasNext()) {
            buf.append(i.next());            
        }
        assertEquals("", buf.toString());        
    }    
}
