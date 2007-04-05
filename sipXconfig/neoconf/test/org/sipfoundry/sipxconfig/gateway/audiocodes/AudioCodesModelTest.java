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
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import junit.framework.TestCase;

public class AudioCodesModelTest extends TestCase {

    public void testCleanSerialNumber() {
        AudioCodesModel model = new AudioCodesModel();
        assertNull(model.cleanSerialNumber(null));
        assertEquals("FT0123456", model.cleanSerialNumber("FT0123456"));
        assertEquals("FT0123456", model.cleanSerialNumber("ft0123456"));
    }
}
