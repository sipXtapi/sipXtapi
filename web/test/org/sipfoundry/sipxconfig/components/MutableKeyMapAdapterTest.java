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
package org.sipfoundry.sipxconfig.components;

import java.util.Iterator;
import java.util.Map;
import java.util.TreeMap;

import junit.framework.TestCase;

public class MutableKeyMapAdapterTest extends TestCase {
    
    Object token1 = new Object();          
    Object token2 = new Object();      
    Object token3 = new Object();
    
    // tree map ensure order is preserved and so unittests
    // can make some assumptions so writing is a little easier
    Map map = new TreeMap();
    
    MutableKeyMapAdapter adapter = new MutableKeyMapAdapter();

    protected void setUp() {
        map.put("token1", token1);
        map.put("token2", token1);
        map.put("token3", token1);
        adapter.setMap(map);
    }
    
    public void testSetCurrentEntry() {        
        Iterator i = adapter.getEntries().iterator();
        adapter.setCurrentEntry(i.next());
        assertSame("token1", adapter.getKey());
        assertSame(token1, adapter.getValue());
        assertEquals("token1", adapter.getCurrentEntry());
    }
    
    public void testSetValue() {        
        Iterator i = adapter.getEntries().iterator();
        adapter.setCurrentEntry(i.next());

        Object testToken = new Object();
        adapter.setValue(testToken);
        assertSame("token1", adapter.getKey());
        assertSame(testToken, adapter.getValue());
        assertSame(testToken, map.get("token1"));
    }

    public void testSetKey() {        
        Iterator i = adapter.getEntries().iterator();
        adapter.setCurrentEntry(i.next());

        adapter.setKey("testkey");        
        assertSame(token1, adapter.getValue());
        assertSame(token1, map.get("testkey"));
        assertNull(map.get("token1"));
    }
}

