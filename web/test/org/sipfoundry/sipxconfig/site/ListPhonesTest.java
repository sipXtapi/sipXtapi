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

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import junit.framework.TestCase;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.test.AbstractInstantiator;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.phone.Endpoint;
import org.sipfoundry.sipxconfig.phone.GenericPhone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneDao;

public class ListPhonesTest extends TestCase {

    private final static String MAC_ADDRESS = "ffffffffff";
    
    public void testListPhones() {
        PhoneContext phoneContext = SiteTestHelper.getPhoneContext();
        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycleControl.replay();
        
        // page w/generated abstract methods implemented
        AbstractInstantiator pageMaker = new AbstractInstantiator();
        ListPhones page = (ListPhones) pageMaker.getInstance(ListPhones.class);
        
        MockControl daoControl = MockControl.createControl(PhoneDao.class);
        PhoneDao dao = (PhoneDao) daoControl.getMock();
        List summaries = new ArrayList();
        Endpoint endpoint = new Endpoint();
        endpoint.setSerialNumber(MAC_ADDRESS);
        endpoint.setPhoneId(GenericPhone.GENERIC_PHONE_ID);
        PhoneListRow summary = new PhoneListRow();
        summary.setPhone(phoneContext.getPhone(endpoint));
        summaries.add(summary);
        daoControl.expectAndReturn(dao.loadPhoneSummaries(page), summaries);
        daoControl.replay();
        
        phoneContext.setPhoneDao(dao);

        page.setPhoneContext(phoneContext);
        page.pageBeginRender(new PageEvent(page, cycle));
        List phones = page.getPhones(); 
        assertNotNull(phones);
        Iterator iphones = phones.iterator();
        assertTrue(iphones.hasNext());
        PhoneListRow firstPhone = (PhoneListRow) iphones.next();
        assertEquals(endpoint, firstPhone.getPhone().getEndpoint());
        assertFalse(iphones.hasNext());
        
        daoControl.verify();
        cycleControl.verify();
    }
}
