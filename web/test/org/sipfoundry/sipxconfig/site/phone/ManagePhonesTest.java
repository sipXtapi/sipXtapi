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
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneSummary;

public class ManagePhonesTest extends TestCase {

    private final static String MAC_ADDRESS = "ffffffffff";
    
    public void testListPhones() {        
        // page w/generated abstract methods implemented        
        MockControl daoControl = MockControl.createStrictControl(PhoneContext.class);
        PhoneContext dao = (PhoneContext) daoControl.getMock();
        List summaries = new ArrayList();
        Phone phone = new GenericPhone();
        Endpoint endpoint = new Endpoint();
        endpoint.setSerialNumber(MAC_ADDRESS);
        endpoint.setPhoneId(phone.getModelId());
        PhoneSummary summary = new PhoneSummary();        
        summary.setPhone(phone);
        summary.setEndpoint(endpoint);
        summaries.add(summary);
        daoControl.expectAndReturn(dao.loadPhoneSummaries(), summaries);
        daoControl.replay();
        
        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycleControl.expectAndReturn(cycle.getAttribute(PhoneContext.CONTEXT_BEAN_NAME), dao);
        cycleControl.replay();

        AbstractInstantiator pageMaker = new AbstractInstantiator();
        ManagePhones page = (ManagePhones) pageMaker.getInstance(ManagePhones.class);
        page.pageBeginRender(new PageEvent(page, cycle));
        List phones = page.getPhones(); 
        assertNotNull(phones);
        Iterator iphones = phones.iterator();
        assertTrue(iphones.hasNext());
        PhoneSummary firstPhone = (PhoneSummary) iphones.next();
        assertEquals(endpoint, firstPhone.getEndpoint());
        assertFalse(iphones.hasNext());
        
        daoControl.verify();
        cycleControl.verify();
    }
}
