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
package org.sipfoundry.sipxconfig.resource;

import java.io.ByteArrayInputStream;
import java.io.InputStream;

import junit.framework.Assert;

public class ResourceTestHelper {
    
    public static final InputStream RETURN_STREAM = new ByteArrayInputStream(new byte[0]);

    public static final InputStream ACCEPT_STREAM = new ByteArrayInputStream(new byte[0]);

    public static void testPutNotAllowed(ResourceHandler handler) {
        try {
            handler.putResource(null, null);
            Assert.fail();
        } catch (MethodNotAllowedException e) {
            Assert.assertTrue(true);
        }        
    }

    public static void testOnResourceNotAllowed(ResourceHandler handler) {
        try {
            handler.onResource(null, "ANY", null);
            Assert.fail();
        } catch (MethodNotAllowedException e) {
            Assert.assertTrue(true);
        }        
    }
    
    public static void testDeleteNotAllowed(ResourceHandler handler) {
        try {
            handler.deleteResource(null, null);
            Assert.fail();
        } catch (MethodNotAllowedException e) {
            Assert.assertTrue(true);
        }        
    }

    public static void testPostNotAllowed(ResourceHandler handler) {
        try {
            handler.postResource(null, null);
            Assert.fail();
        } catch (MethodNotAllowedException e) {
            Assert.assertTrue(true);
        }        
    }

}
