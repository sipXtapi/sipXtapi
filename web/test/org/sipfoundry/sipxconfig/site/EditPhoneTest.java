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
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.test.AbstractInstantiator;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.GenericPhone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneDao;

public class EditPhoneTest extends TestCase {

    public void testSave() {
        PhoneContext phoneContext = SiteTestHelper.getPhoneContext();
        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycle.activate("ListPhones");
        cycleControl.replay();

        AbstractInstantiator pageMaker = new AbstractInstantiator();
        EditPhone page = (EditPhone) pageMaker.getInstance(EditPhone.class);
        page.setPhoneContext(phoneContext);
        page.setEndpoint(new Endpoint());
        page.pageBeginRender(new PageEvent(page, cycle));
        page.getEndpoint().setSerialNumber(Long.toHexString(new Date().getTime()));
        page.getEndpoint().setPhoneId(GenericPhone.GENERIC_PHONE_ID);

        MockControl daoControl = MockControl.createStrictControl(PhoneDao.class);
        PhoneDao dao = (PhoneDao) daoControl.getMock();
        phoneContext.setPhoneDao(dao);
        dao.storeEndpoint(page.getEndpoint());
        daoControl.replay();

        IPropertySelectionModel ids = page.getPhoneSelectionModel();
        assertTrue(0 < ids.getOptionCount());
        assertEquals(GenericPhone.GENERIC_PHONE_ID, ids.getOption(0));

        page.save(cycle);

        daoControl.verify();
        cycleControl.verify();
    }

    public void textCancel() {
        PhoneContext phoneContext = SiteTestHelper.getPhoneContext();
        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycle.activate("ListPhones");
        cycleControl.replay();

        AbstractInstantiator pageMaker = new AbstractInstantiator();
        EditPhone page = (EditPhone) pageMaker.getInstance(EditPhone.class);
        page.setEndpoint(new Endpoint());
        page.setPhoneContext(phoneContext);
        page.pageBeginRender(new PageEvent(page, cycle));
        page.cancel(cycle);

        cycleControl.verify();
    }

}
