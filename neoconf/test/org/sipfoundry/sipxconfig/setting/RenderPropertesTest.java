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
package org.sipfoundry.sipxconfig.setting;

import junit.framework.TestCase;

public class RenderPropertesTest extends TestCase {
    
    public void testPasswordField() {
        assertFalse(new RenderProperties().isPassword());
        assertTrue(RenderProperties.createPasswordField().isPassword());
    }
}
