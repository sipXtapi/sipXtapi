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

import java.util.Date;

import junit.framework.TestCase;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.test.AbstractInstantiator;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.GenericPhone;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneDao;

public class EditPhoneTest extends TestCase {

    public void testSave() {
        SiteTestHelper helper = SiteTestHelper.createHelper();
        PhoneContext phoneContext = helper.getPhoneContext();
        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycle.activate("ListPhones");
        cycleControl.replay();

        AbstractInstantiator pageMaker = new AbstractInstantiator();
        EditPhone page = (EditPhone) pageMaker.getInstance(EditPhone.class);
        Endpoint endpoint = new Endpoint();
        endpoint.setPhoneId(GenericPhone.GENERIC_PHONE_ID);
        Phone phone = phoneContext.getPhone(endpoint);
        assertNotNull(phone);
        page.setPhone(phone);
        endpoint.setSerialNumber(Long.toHexString(new Date().getTime()));
        MockControl daoControl = MockControl.createStrictControl(PhoneDao.class);
        PhoneDao dao = (PhoneDao) daoControl.getMock();
        phoneContext.setPhoneDao(dao);
        dao.storeEndpoint(endpoint);
        daoControl.replay();

        page.ok(cycle);

        daoControl.verify();
        cycleControl.verify();
    }

    public void textCancel() {
        SiteTestHelper.createHelper();
        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycle.activate("ListPhones");
        cycleControl.replay();

        AbstractInstantiator pageMaker = new AbstractInstantiator();
        EditPhone page = (EditPhone) pageMaker.getInstance(EditPhone.class);
        page.cancel(cycle);

        cycleControl.verify();
    }

}
