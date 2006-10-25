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
package org.sipfoundry.sipxconfig.phonebook;

import java.io.File;
import java.util.Collection;
import java.util.Collections;

import junit.framework.TestCase;

import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.setting.Group;

public class PhonebookManagerTest extends TestCase {
    
    public void testMkdirs() {
        String dir = String.valueOf(System.currentTimeMillis());
        assertFalse(new File(dir).exists());
        PhonebookManagerImpl.mkdirs(dir);
        assertTrue(new File(dir).exists());
    }
    
    public void testGetEmptyPhonebookRows() {
        PhonebookManagerImpl context = new PhonebookManagerImpl();
        assertEquals(0, context.getRows(new Phonebook()).size());
    }
    
    public void testGetRows() {       
        Phonebook phonebook = new Phonebook();
        Group group = new Group();
        User user = new User();
        user.setFirstName("Tweety");
        user.setLastName("Bird");
        user.setUserName("tbird");
        phonebook.setMembers(Collections.singleton(group));
        Collection<User> users = Collections.singleton(user);
        IMocksControl coreContextControl = EasyMock.createControl();
        CoreContext coreContext = coreContextControl.createMock(CoreContext.class);
        coreContext.getGroupMembers(group);
        coreContextControl.andReturn(users);
        coreContextControl.replay();
        
        PhonebookManagerImpl context = new PhonebookManagerImpl();
        context.setCoreContext(coreContext);              
        Collection<PhonebookEntry> entries = context.getRows(phonebook);
        assertEquals(1, entries.size());
        PhonebookEntry entry = entries.iterator().next(); 
        assertEquals("Tweety", entry.getFirstName());
        
        coreContextControl.verify();
    }
}
