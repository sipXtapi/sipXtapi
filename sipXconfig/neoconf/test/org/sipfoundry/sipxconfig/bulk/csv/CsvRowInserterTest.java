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

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.cisco.CiscoModel;
import org.sipfoundry.sipxconfig.phone.polycom.PolycomModel;

public class CsvRowInserterTest extends TestCase {
    public void testUserFromRow() {
        User bongo = new User();
        bongo.setUserName("bongo");
        bongo.setFirstName("Ringo");

        IMocksControl coreContextCtrl = EasyMock.createControl();
        CoreContext coreContext = coreContextCtrl.createMock(CoreContext.class);
        coreContext.loadUserByUserName("bongo");
        coreContextCtrl.andReturn(bongo);
        coreContext.loadUserByUserName("kuku");
        coreContextCtrl.andReturn(null);
        coreContext.getAuthorizationRealm();
        coreContextCtrl.andReturn("sipfoundry.org").times(2);

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

        IMocksControl phoneContextCtrl = EasyMock.createControl();
        PhoneContext phoneContext = phoneContextCtrl.createMock(PhoneContext.class);

        phoneContext.getPhoneIdBySerialNumber("001122334466");
        phoneContextCtrl.andReturn(phoneId);
        phoneContext.loadPhone(phoneId);
        phoneContextCtrl.andReturn(phone);

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

        IMocksControl phoneContextCtrl = EasyMock.createControl();
        PhoneContext phoneContext = phoneContextCtrl.createMock(PhoneContext.class);

        phoneContext.getPhoneIdBySerialNumber("001122334455");
        phoneContextCtrl.andReturn(null);
        phoneContext.newPhone(PolycomModel.MODEL_500);
        phoneContextCtrl.andReturn(phone);

        phoneContextCtrl.replay();

        CsvRowInserter impl = new CsvRowInserter();
        impl.setPhoneContext(phoneContext);

        // new phone
        Phone phone1 = impl.phoneFromRow(phoneRow1);
        assertEquals("phone in John room", phone1.getDescription());
        assertEquals("001122334455", phone1.getSerialNumber());

        phoneContextCtrl.verify();
    }
    
    public void testPhoneFromRowSpaces() {
        final String[] phoneRow1 = new String[] {
            "", "", "", "", "", "", "", "001122334455", "ciscoAta ", " 18x", "yellow phone",
            "phone in John room"
        };

        Phone phone = new Phone();
        phone.setDescription("old description");

        IMocksControl phoneContextCtrl = EasyMock.createControl();
        PhoneContext phoneContext = phoneContextCtrl.createMock(PhoneContext.class);

        phoneContext.getPhoneIdBySerialNumber("001122334455");
        phoneContextCtrl.andReturn(null);
        phoneContext.newPhone(CiscoModel.MODEL_ATA18X);
        phoneContextCtrl.andReturn(phone);

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
