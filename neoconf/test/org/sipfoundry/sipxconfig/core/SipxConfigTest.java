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
package org.sipfoundry.sipxconfig.core;

import junit.framework.TestCase;

import org.springframework.context.support.ClassPathXmlApplicationContext;

/**
 * Comments
 */
public class SipxConfigTest extends TestCase {
    
    public void testInstantiation() {
        ClassPathXmlApplicationContext ctx = new ClassPathXmlApplicationContext(
                "org/sipfoundry/sipxconfig/core/applicationContext-device.xml");
        SipxConfig sipx = (SipxConfig) ctx.getBean("sipxconfig");
        assertNotNull(sipx);               
    }
}
