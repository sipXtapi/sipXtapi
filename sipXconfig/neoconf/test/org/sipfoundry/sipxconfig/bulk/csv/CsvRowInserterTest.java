/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.bulk.csv;

import junit.framework.TestCase;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.polycom.PolycomModel;

public class CsvRowInserterTest extends TestCase {
    public void testUserFromRow() {
        User bongo = new User();
        bongo.setUserName("bongo");
        bongo.setFirstName("Ringo");

        MockControl coreContextCtrl = MockControl.createControl(CoreContext.class);

        CoreContext coreContext = (CoreContext) coreContextCtrl.getMock();
        coreContext.loadUserByUserName("bongo");
        coreContextCtrl.setReturnValue(bongo);
        coreContext.loadUserByUserName("kuku");
        coreContextCtrl.setReturnValue(null);
        coreContext.getAuthorizationRealm();
        coreContextCtrl.setReturnValue("sipfoundry.org", 2);

        coreContextCtrl.replay();

        String[] userRow1 = new String[] {
            "bongo", "1234", "abcdef", "", "Star", ""
        };

        String[] userRow2 = new String[] {
            "kuku", "1234", "abcdef", "John", "Lennon", "jlennon, 121212"
        };

        CsvRowInserter impl = new CsvRowInserter();
        impl.setCoreContext(coreContext);

        User user1 = impl.userFromRow(userRow1);
        assertEquals("bongo", user1.getUserName());
        assertEquals("Ringo", user1.getFirstName());
        assertEquals("Star", user1.getLastName());
        assertEquals("abcdef", user1.getSipPassword());
        assertEquals(32, user1.getPintoken().length());

        User user2 = impl.userFromRow(userRow2);
        assertEquals("kuku", user2.getUserName());
        assertEquals("John", user2.getFirstName());
        assertEquals("Lennon", user2.getLastName());
        assertEquals("abcdef", user2.getSipPassword());
        assertEquals(32, user2.getPintoken().length());
        assertEquals("jlennon, 121212", user2.getAliasesString());

        coreContextCtrl.verify();
    }

    public void testPhoneFromRowUpdate() {
        final String[] phoneRow = new String[] {
            "", "", "", "", "", "", "", "001122334466", "polycom", "300", "yellow phone", ""
        };

        Integer phoneId = new Integer(5);
        Phone phone = new Phone();
        phone.setSerialNumber("001122334466");
        phone.setDescription("old description");

        MockControl phoneContextCtrl = MockControl.createControl(PhoneContext.class);
        PhoneContext phoneContext = (PhoneContext) phoneContextCtrl.getMock();

        phoneContext.getPhoneIdBySerialNumber("001122334466");
        phoneContextCtrl.setReturnValue(phoneId);
        phoneContext.loadPhone(phoneId);
        phoneContextCtrl.setReturnValue(phone);

        phoneContextCtrl.replay();

        CsvRowInserter impl = new CsvRowInserter();
        impl.setPhoneContext(phoneContext);

        // update existing phone
        Phone phone1 = impl.phoneFromRow(phoneRow);
        assertEquals("old description", phone1.getDescription());
        assertEquals("001122334466", phone1.getSerialNumber());

        phoneContextCtrl.verify();
    }

    public void testPhoneFromRowNew() {
        final String[] phoneRow1 = new String[] {
            "", "", "", "", "", "", "", "001122334455", "polycom", "500", "yellow phone",
            "phone in John room"
        };

        Phone phone = new Phone();
        phone.setDescription("old description");

        MockControl phoneContextCtrl = MockControl.createControl(PhoneContext.class);
        PhoneContext phoneContext = (PhoneContext) phoneContextCtrl.getMock();

        phoneContext.getPhoneIdBySerialNumber("001122334455");
        phoneContextCtrl.setReturnValue(null);
        phoneContext.newPhone(PolycomModel.MODEL_500);
        phoneContextCtrl.setReturnValue(phone);

        phoneContextCtrl.replay();

        CsvRowInserter impl = new CsvRowInserter();
        impl.setPhoneContext(phoneContext);

        // new phone
        Phone phone1 = impl.phoneFromRow(phoneRow1);
        assertEquals("phone in John room", phone1.getDescription());
        assertEquals("001122334455", phone1.getSerialNumber());

        phoneContextCtrl.verify();
    }
}
