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

import org.apache.commons.lang.enums.Enum;
import org.apache.hivemind.Messages;
import org.easymock.MockControl;

public class EnumFormatTest extends TestCase {

    public void testFormat() throws Exception {
        EnumFormat format = new EnumFormat();
        assertEquals("bongo", format.format(FakeEnum.BONGO));
        assertEquals("kuku", format.format(FakeEnum.KUKU));
    }

    public void testLolizedFormat() throws Exception {
        MockControl messagesCtrl = MockControl.createControl(Messages.class);
        Messages messages = (Messages) messagesCtrl.getMock();
        messages.getMessage("fake.bongo");
        messagesCtrl.setReturnValue("localized bongo");
        messages.getMessage("fake.kuku");
        messagesCtrl.setReturnValue("localized kuku");
        messagesCtrl.replay();

        EnumFormat format = new EnumFormat();
        format.setMessages(messages);
        format.setPrefix("fake");

        assertEquals("localized bongo", format.format(FakeEnum.BONGO));
        assertEquals("localized kuku", format.format(FakeEnum.KUKU));
        messagesCtrl.verify();
    }

    static class FakeEnum extends Enum {
        public static final FakeEnum BONGO = new FakeEnum("bongo");
        public static final FakeEnum KUKU = new FakeEnum("kuku");

        private FakeEnum(String name) {
            super(name);
        }
    }
}
