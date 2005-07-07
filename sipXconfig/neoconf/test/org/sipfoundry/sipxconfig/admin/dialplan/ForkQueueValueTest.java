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
package org.sipfoundry.sipxconfig.admin.dialplan;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.admin.forwarding.Ring;

/**
 * ForkQueueValueTest
 */
public class ForkQueueValueTest extends TestCase {
    
    public void testSerial() throws Exception {
        ForkQueueValue value = new ForkQueueValue(3);
        assertEquals("q=0.95",value.getSerial());
        assertEquals("q=0.9",value.getSerial());
        assertEquals("q=0.85",value.getSerial());        
    }

    public void testMixed() throws Exception {
        ForkQueueValue value = new ForkQueueValue(3);
        assertEquals("q=1.0",value.getParallel());
        assertEquals("q=1.0",value.getParallel());
        assertEquals("q=0.95",value.getSerial());
        assertEquals("q=0.95",value.getParallel());
        assertEquals("q=0.9",value.getSerial());
        assertEquals("q=0.85",value.getSerial());        
    }

    public void testLastBeforeNext() throws Exception {
        ForkQueueValue value = new ForkQueueValue(3);
        assertEquals("q=1.0",value.getParallel());
        assertEquals("q=1.0",value.getParallel());
        assertEquals("q=0.95",value.getSerial());
        assertEquals("q=0.95",value.getParallel());
        assertEquals("q=0.9",value.getSerial());
        assertEquals("q=0.9",value.getParallel());
    }
    
    public void testGetValue() {
        ForkQueueValue value = new ForkQueueValue(3);
        assertEquals("q=1.0",value.getValue(Ring.Type.IMMEDIATE));
        assertEquals("q=1.0",value.getValue(Ring.Type.IMMEDIATE));
        assertEquals("q=0.95",value.getValue(Ring.Type.DELAYED));
        assertEquals("q=0.95",value.getValue(Ring.Type.IMMEDIATE));
        assertEquals("q=0.9",value.getValue(Ring.Type.DELAYED));
        assertEquals("q=0.85",value.getValue(Ring.Type.DELAYED));                
    }
}
