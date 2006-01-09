/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.bulk;

import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.StringReader;

import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.sipfoundry.sipxconfig.phone.PhoneModel;
import org.springframework.context.ApplicationContext;

public class BulkManagerImplTestDb extends SipxDatabaseTestCase {
    private BulkManager m_bulkManager;

    protected void setUp() throws Exception {
        super.setUp();
        ApplicationContext context = TestHelper.getApplicationContext();
        m_bulkManager = (BulkManager) context.getBean("bulkManagerDao");
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testInsertFromCsvEmpty() throws Exception {
        m_bulkManager.insertFromCsv(new StringReader(""));
        assertEquals(0, getConnection().getRowCount("users"));
        assertEquals(0, getConnection().getRowCount("phone"));
        assertEquals(0, getConnection().getRowCount("line"));
        assertEquals(0, getConnection().getRowCount("user_group"));
        assertEquals(0, getConnection().getRowCount("phone_group"));
    }

    public void testInsertFromCsvNameDuplication() throws Exception {
        // users with duplicated names should be overwritten
        InputStream cutsheet = getClass().getResourceAsStream("dup_names.csv");
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet));
        assertEquals(2, getConnection().getRowCount("users"));
        assertEquals(3, getConnection().getRowCount("phone"));
        assertEquals(3, getConnection().getRowCount("line"));
        assertEquals(2, getConnection().getRowCount("user_group"));
        assertEquals(3, getConnection().getRowCount("phone_group"));
        assertEquals(1, getConnection().getRowCount("group_storage", "where resource = 'phone'"));
        assertEquals(2, getConnection().getRowCount("group_storage", "where resource = 'user'"));
    }

    public void testInsertFromCsvAliasDuplication() throws Exception {
        // second user has a duplicated alias - it should be ignored, but remaining users have to
        // be imported
        InputStream cutsheet = getClass().getResourceAsStream("errors.csv");
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet));
        assertEquals(2, getConnection().getRowCount("users"));
        assertEquals(2, getConnection().getRowCount("phone"));
        assertEquals(2, getConnection().getRowCount("line"));
        assertEquals(2, getConnection().getRowCount("user_group"));
        assertEquals(2, getConnection().getRowCount("phone_group"));
        assertEquals(1, getConnection().getRowCount("group_storage", "where resource = 'phone'"));
        assertEquals(2, getConnection().getRowCount("group_storage", "where resource = 'user'"));
    }

    public void testInsertFromCsvPhoneDuplication() throws Exception {
        // users with duplicated names should be overwritten
        InputStream cutsheet = getClass().getResourceAsStream("dup_phones.csv");
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet));
        assertEquals(5, getConnection().getRowCount("users"));
        assertEquals(4, getConnection().getRowCount("phone"));
        assertEquals(5, getConnection().getRowCount("line"));
        assertEquals(5, getConnection().getRowCount("user_group"));
        assertEquals(4, getConnection().getRowCount("phone_group"));
        assertEquals(2, getConnection().getRowCount("group_storage", "where resource = 'phone'"));
        assertEquals(2, getConnection().getRowCount("group_storage", "where resource = 'user'"));
    }
    
    public void testInsertFromCsv() throws Exception {
        InputStream cutsheet = getClass().getResourceAsStream("cutsheet.csv");
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet));
        assertEquals(5, getConnection().getRowCount("users"));
        assertEquals(5, getConnection().getRowCount("phone"));
        assertEquals(5, getConnection().getRowCount("line"));
        assertEquals(5, getConnection().getRowCount("user_group"));
        assertEquals(5, getConnection().getRowCount("phone_group"));
        assertEquals(2, getConnection().getRowCount("group_storage", "where resource = 'phone'"));
        assertEquals(2, getConnection().getRowCount("group_storage", "where resource = 'user'"));
    }

    public void testInsertFromCsvDuplicate() throws Exception {
        InputStream cutsheet = getClass().getResourceAsStream("cutsheet.csv");
        cutsheet.mark(-1);
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet));
        // and try again
        cutsheet.reset();
        m_bulkManager.insertFromCsv(new InputStreamReader(cutsheet));
        assertEquals(5, getConnection().getRowCount("users"));
        assertEquals(5, getConnection().getRowCount("phone"));
        // lines are re-added for now - everything else is updated
        assertEquals(10, getConnection().getRowCount("line"));
        assertEquals(5, getConnection().getRowCount("user_group"));
        assertEquals(5, getConnection().getRowCount("phone_group"));
        assertEquals(2, getConnection().getRowCount("group_storage", "where resource = 'phone'"));
        assertEquals(2, getConnection().getRowCount("group_storage", "where resource = 'user'"));
    }
    

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

        BulkManagerImpl impl = new BulkManagerImpl();
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

        BulkManagerImpl impl = new BulkManagerImpl();
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
        phoneContext.newPhone(PhoneModel.getModel("polycom", "500"));
        phoneContextCtrl.setReturnValue(phone);

        phoneContextCtrl.replay();

        BulkManagerImpl impl = new BulkManagerImpl();
        impl.setPhoneContext(phoneContext);

        // new phone
        Phone phone1 = impl.phoneFromRow(phoneRow1);
        assertEquals("phone in John room", phone1.getDescription());
        assertEquals("001122334455", phone1.getSerialNumber());

        phoneContextCtrl.verify();
    }
}
