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

public class ListPhonesTest extends TestCase {

    private final static String MAC_ADDRESS = "ffffffffff";
    
    public void testListPhones() {
        /*
        SiteTestHelper.initTapestryUtils();
        MockControl cycleControl = MockControl.createStrictControl(IRequestCycle.class);
        IRequestCycle cycle = (IRequestCycle) cycleControl.getMock();
        cycleControl.replay();
        
        // page w/generated abstract methods implemented
        AbstractInstantiator pageMaker = new AbstractInstantiator();
        ManagePhones page = (ManagePhones) pageMaker.getInstance(ManagePhones.class);
        
        MockControl daoControl = MockControl.createControl(PhoneDao.class);
        PhoneDao dao = (PhoneDao) daoControl.getMock();
        List summaries = new ArrayList();
        Endpoint endpoint = new Endpoint();
        endpoint.setSerialNumber(MAC_ADDRESS);
        endpoint.setPhoneId(GenericPhone.GENERIC_PHONE_ID);
        PhoneSummary summary = new PhoneSummary();
        summary.setPhone(phoneContext.getPhone(endpoint));
        summaries.add(summary);
        daoControl.expectAndReturn(dao.loadPhoneSummaries(phoneContext), summaries);
        daoControl.replay();
        
        phoneContext.setPhoneContext(dao);

        page.pageBeginRender(new PageEvent(page, cycle));
        List phones = page.getPhones(); 
        assertNotNull(phones);
        Iterator iphones = phones.iterator();
        assertTrue(iphones.hasNext());
        PhoneSummary firstPhone = (PhoneSummary) iphones.next();
        assertEquals(endpoint, firstPhone.getPhone().getEndpoint());
        assertFalse(iphones.hasNext());
        
        daoControl.verify();
        cycleControl.verify();
        */
    }
}
