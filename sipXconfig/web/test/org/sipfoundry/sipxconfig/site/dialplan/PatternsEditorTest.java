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
package org.sipfoundry.sipxconfig.site.dialplan;

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.dialplan.DialPattern;

import junit.framework.TestCase;

public class PatternsEditorTest extends TestCase {
    public void testSetCollectionSize() {
        List patterns = new ArrayList();
        PatternsEditor.setCollectionSize(patterns, 2);
        assertEquals(2, patterns.size());
        PatternsEditor.setCollectionSize(patterns, 0);
        assertEquals(0, patterns.size());
        PatternsEditor.setCollectionSize(patterns, 3);
        assertEquals(3, patterns.size());
        Object pattern = patterns.get(0);
        assertTrue(pattern instanceof DialPattern);
        PatternsEditor.setCollectionSize(patterns, 1);        
        assertEquals(1, patterns.size());
        assertSame(pattern, patterns.get(0));
        PatternsEditor.setCollectionSize(patterns, 2);
        assertEquals(2, patterns.size());
        assertSame(pattern, patterns.get(0));
    }
}
