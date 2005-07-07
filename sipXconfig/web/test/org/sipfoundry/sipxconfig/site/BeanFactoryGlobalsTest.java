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
package org.sipfoundry.sipxconfig.site;

import java.util.HashMap;
import java.util.Map;

import junit.framework.TestCase;
import ognl.Ognl;

import org.easymock.MockControl;
import org.springframework.web.context.WebApplicationContext;

/**
 * BeanFactoryGlobalsTest
 */
public class BeanFactoryGlobalsTest extends TestCase {

    public void testGetApplicationContext() throws Exception {
        Object testValue = new Integer(777);

        BeanFactoryGlobals globals = new BeanFactoryGlobals();

        MockControl control = MockControl.createStrictControl(WebApplicationContext.class);
        control.setDefaultMatcher(MockControl.EQUALS_MATCHER);
        WebApplicationContext wac = (WebApplicationContext) control.getMock();
        control.expectAndReturn(wac.getBean("bongo"), testValue);
        control.replay();

        // test direct access to application context
        globals.setApplicationContext(wac);
        assertSame(wac, globals.getApplicationContext());

        Map ognlRoot = new HashMap();
        ognlRoot.put("global", globals);

        // test OGNL access direct access to application context
        Object value = Ognl.getValue("global.getApplicationContext()", ognlRoot);
        assertSame(wac, value);

        // test OGNL access to Spring beans
        value = Ognl.getValue("global.bongo", ognlRoot);
        assertEquals(testValue, value);
        control.verify();
    }
}
