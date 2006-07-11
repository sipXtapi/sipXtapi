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
package org.taha.interceptor;

import junit.framework.TestCase;

public class MethodCacheInterceptorTest extends TestCase {
    
    public void testGetCacheKey() {
        assertEquals("a", MethodCacheInterceptor.getCacheKey("a"));
        assertEquals("[a,b,]", MethodCacheInterceptor.getCacheKey(new String[] {"a", "b"}));
        assertEquals("[a,[b,c,],]", MethodCacheInterceptor.getCacheKey(new Object[] {"a", 
                new Object[] { "b", "c" }}));        
    }
}
