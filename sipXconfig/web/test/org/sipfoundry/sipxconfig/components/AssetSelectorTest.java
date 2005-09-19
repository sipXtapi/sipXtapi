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
package org.sipfoundry.sipxconfig.components;

import junit.framework.TestCase;

public class AssetSelectorTest extends TestCase {

    public void testGetSystemIndependentFileName() {
        assertEquals("kuku.txt", AssetSelector.getSystemIndependentFileName("kuku.txt"));
        assertEquals("kuku.txt", AssetSelector.getSystemIndependentFileName("/kuku.txt"));
        assertEquals("kuku.txt", AssetSelector.getSystemIndependentFileName("c:kuku.txt"));
        assertEquals("kuku.txt", AssetSelector.getSystemIndependentFileName("c:\\dir\\kuku.txt"));
        assertEquals("kuku.txt", AssetSelector.getSystemIndependentFileName("dir/kuku.txt"));
        assertEquals("kuku.txt", AssetSelector.getSystemIndependentFileName("/d\\ir/kuku.txt"));
        assertEquals("", AssetSelector.getSystemIndependentFileName(""));
        assertEquals("", AssetSelector.getSystemIndependentFileName(null));
    }
}
