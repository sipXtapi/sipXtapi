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

public class EditPhoneTest extends TestCase {

    public void testSave() {
        /*
        SiteTestHelper.initTapestryUtils();
        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycle.activate(ManagePhones.PAGE);
        cycleControl.replay();

        AbstractInstantiator pageMaker = new AbstractInstantiator();
        EditPhone page = (EditPhone) pageMaker.getInstance(EditPhone.class);
        GenericPhone phone = new GenericPhone();
        Endpoint endpoint = new Endpoint();
        phone.setEndpoint(endpoint);
        endpoint.setPhoneId(phone.getModelId());
        page.setPhone(phone);
        endpoint.setSerialNumber(Long.toHexString(new Date().getTime()));
        
        MockControl daoControl = MockControl.createStrictControl(PhoneContext.class);
        PhoneContext dao = (PhoneContext) daoControl.getMock();
        dao.storeEndpoint(endpoint);
        daoControl.replay();

        page.ok(cycle);

        daoControl.verify();
        cycleControl.verify();
        */
    }

    public void textCancel() {
        /*
        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycle.activate("ListPhones");
        cycleControl.replay();

        AbstractInstantiator pageMaker = new AbstractInstantiator();
        EditPhone page = (EditPhone) pageMaker.getInstance(EditPhone.class);
        page.cancel(cycle);

        cycleControl.verify();
        */
    }

}
