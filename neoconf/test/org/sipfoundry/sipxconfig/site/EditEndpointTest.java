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
package org.sipfoundry.sipxconfig.site;

import junit.framework.TestCase;

/**
 * Comments
 */
public class EditEndpointTest extends TestCase {
    
    public void testSave() {
        EditEndpoint editEndpoint = new EditEndpoint();
        assertEquals("success", editEndpoint.save());
    }
}
