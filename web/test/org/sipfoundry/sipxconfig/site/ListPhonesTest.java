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
import org.apache.tapestry.contrib.table.model.ITableModel;
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
        
        MockControl daoControl = MockControl.createControl(PhoneDao.class);
        PhoneDao dao = (PhoneDao) daoControl.getMock();
        List endpoints = new ArrayList();
        Endpoint endpoint = new Endpoint();
        endpoint.setSerialNumber(MAC_ADDRESS);
        endpoint.setPhoneId(GenericPhone.GENERIC_PHONE_ID);
        endpoints.add(endpoint);
        daoControl.expectAndReturn(dao.loadEndpoints(), endpoints);
        daoControl.replay();
        
        phoneContext.setPhoneDao(dao);

        ListPhones page = new ListPhones();
        page.setPhoneContext(phoneContext);
        ITableModel model = page.getTableModel(); 
        assertEquals(1, model.getPageCount());
        Iterator rows = model.getCurrentPageRows();
        assertTrue(rows.hasNext());
        assertEquals(endpoint, rows.next());
        
        daoControl.verify();
        cycleControl.verify();
    }
}
