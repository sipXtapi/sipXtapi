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
package org.sipfoundry.sipxconfig.components;

import junit.framework.TestCase;

import org.apache.tapestry.IRequestCycle;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.GenericPhone;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneDao;
import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.phone.PhonePageUtils;

/**
 * Comments
 */
public class PhonePageUtilsTest extends TestCase {
    
    public void testGetPhoneContext() {
        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycleControl.replay();

        // setups up spring
        SiteTestHelper.createHelper();
        
        assertNotNull(PhonePageUtils.getPhoneContext(cycle));
    }
    
    public void testGetPhoneFromParameter() {
        // setups up spring
        SiteTestHelper helper = SiteTestHelper.createHelper();
        
        MockControl daoControl = MockControl.createStrictControl(PhoneDao.class);
        PhoneDao dao = (PhoneDao) daoControl.getMock();
        Endpoint endpoint = new Endpoint();
        endpoint.setPhoneId(GenericPhone.GENERIC_PHONE_ID);
        daoControl.expectAndReturn(dao.loadEndpoint(endpoint.getId()), endpoint);
        daoControl.replay();
        helper.getPhoneContext().setPhoneDao(dao);
        
        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        Object[] params = new Object[] { 
                new Integer(endpoint.getId()) 
            };
        cycleControl.expectAndReturn(cycle.getServiceParameters(), params);
        cycleControl.replay();
        
        // setups up spring
        SiteTestHelper.createHelper();

        Phone phone = PhonePageUtils.getPhoneFromParameter(cycle, 0);
        assertEquals(GenericPhone.GENERIC_PHONE_ID, phone.getModelId());
        assertEquals(endpoint, phone.getEndpoint());        
    }
}
