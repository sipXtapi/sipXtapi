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
package org.sipfoundry.sipxconfig.site.phone;

import junit.framework.TestCase;

/**
 * Comments
 */
public class PhonePageUtilsTest extends TestCase {
    
    public void testGetPhoneContext() {
        /*
        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycleControl.replay();

        // setups up spring
        SiteTestHelper.initTapestryUtils();
        
        assertNotNull(PhonePageUtils.getPhoneContext(cycle));
        */
    }
    
    public void testGetPhoneFromParameter() {
        /*
        // setups up spring
        MockControl daoControl = MockControl.createStrictControl(PhoneContext.class);
        PhoneContext dao = (PhoneContext) daoControl.getMock();
        GenericPhone expectedPhone = new GenericPhone();
        Endpoint expectedEndpoint = new Endpoint();
        expectedEndpoint.setPhoneId(GenericPhone.GENERIC_PHONE_ID);
        daoControl.expectAndReturn(dao.loadEndpoint(expectedEndpoint.getId()), expectedEndpoint);
        daoControl.expectAndReturn(dao.getPhone(expectedEndpoint), expectedPhone);
        daoControl.replay();
        
        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        Object[] params = new Object[] { 
                new Integer(expectedEndpoint.getId()) 
            };
        cycleControl.expectAndReturn(cycle.getServiceParameters(), params);
        cycleControl.replay();

        Phone actualPhone = PhonePageUtils.getPhoneFromParameter(cycle, 0);
        assertEquals(expectedPhone, actualPhone);
        assertEquals(expectedEndpoint, actualPhone.getEndpoint());
        */        
    }
}
