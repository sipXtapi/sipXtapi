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
package org.sipfoundry.sipxconfig.setting;

import java.io.File;

import junit.framework.TestCase;

public class ModelMessageSourceTest extends TestCase {
    public void testGetBundle() throws Exception {
        File file = new File("manufacturer/phone.xml");
        ModelMessageSource source = new ModelMessageSource(file);
        // ideally assertEquals(manufacturer.phone, source.basenames[0])
        assertTrue(source.toString().contains("basenames=[manufacturer.phone]"));
    }
}
