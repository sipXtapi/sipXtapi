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

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collection;

import junit.framework.TestCase;

public class SelectionCollectionTest extends TestCase {
    
    SelectionCollection selections;
    
    Collection collection;
    
    String[] items = { "one", "two", "three" };
    
    protected void setUp() {
        selections = new SelectionCollection();
        collection = new ArrayList(Arrays.asList(items));
        selections.setCollection(collection);
    }

    public void testUnSelect() {
        assertTrue(collection.contains("one"));
        selections.setSelected("one", false);
        assertFalse(collection.contains("one"));
        assertEquals(2, collection.size());
    }

    public void testSelect() {
        assertFalse(collection.contains("four"));
        selections.setSelected("four", true);
        assertTrue(collection.contains("four"));
        assertEquals(4, collection.size());
    }
}
