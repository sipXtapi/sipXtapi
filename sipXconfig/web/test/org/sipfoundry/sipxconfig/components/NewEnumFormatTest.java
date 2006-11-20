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
package org.sipfoundry.sipxconfig.components;

import junit.framework.TestCase;

import org.apache.hivemind.Messages;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;

public class NewEnumFormatTest extends TestCase {

    public void testFormat() throws Exception {
        NewEnumFormat format = new NewEnumFormat();
        assertEquals("BONGO", format.format(FakeEnum.BONGO));
        assertEquals("KUKU", format.format(FakeEnum.KUKU));
    }

    public void testLocalizedFormat() throws Exception {
        IMocksControl messagesCtrl = EasyMock.createControl();
        Messages messages = messagesCtrl.createMock(Messages.class);
        messages.getMessage("fake.BONGO");
        messagesCtrl.andReturn("localized bongo");
        messages.getMessage("fake.KUKU");
        messagesCtrl.andReturn("localized kuku");
        messagesCtrl.replay();

        NewEnumFormat format = new NewEnumFormat();
        format.setMessages(messages);
        format.setPrefix("fake");

        assertEquals("localized bongo", format.format(FakeEnum.BONGO));
        assertEquals("localized kuku", format.format(FakeEnum.KUKU));
        messagesCtrl.verify();
    }

    enum FakeEnum {
        BONGO, KUKU
    }    
}
