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

import java.util.Date;

import junit.framework.TestCase;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.test.AbstractInstantiator;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.GenericPhone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;

public class EditPhoneTest extends TestCase {

    public void testSave() {
        AbstractInstantiator pageMaker = new AbstractInstantiator();
        EditPhone page = (EditPhone) pageMaker.getInstance(EditPhone.class);
        Endpoint endpoint = new Endpoint();
        page.setEndpoint(endpoint);
        endpoint.setPhoneId(GenericPhone.GENERIC_PHONE_ID);
        endpoint.setSerialNumber(Long.toHexString(new Date().getTime()));
        
        MockControl daoControl = MockControl.createStrictControl(PhoneContext.class);
        PhoneContext dao = (PhoneContext) daoControl.getMock();
        dao.storeEndpoint(endpoint);
        dao.flush();
        daoControl.replay();

        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycleControl.expectAndReturn(cycle.getAttribute(PhoneContext.CONTEXT_BEAN_NAME), dao);
        cycle.activate(ManagePhones.PAGE);
        cycleControl.replay();

        page.ok(cycle);

        cycleControl.verify();
        daoControl.verify();
    }
}
