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
package org.sipfoundry.sipxconfig.site.vm;

import junit.framework.TestCase;

import org.sipfoundry.sipxconfig.site.vm.MailboxOperation.MalformedMailboxUrlException;
import org.sipfoundry.sipxconfig.site.vm.MailboxOperation.MoveVoiceMail;
import org.sipfoundry.sipxconfig.site.vm.MailboxOperation.PlayVoiceMail;

public class MailboxOperationTest extends TestCase {

    public void testCreateMailboxOperationFromServletPath() {
        MailboxOperation op = MailboxOperation.createMailboxOperationFromServletPath("/userId/folderId");
        assertEquals("userId", op.getUserId());
        assertEquals("folderId", op.getFolderId());
    }

    public void testCreatePlayOperationFromServletPath() {
        MailboxOperation op = MailboxOperation.createMailboxOperationFromServletPath("/userId/folderId/foo");
        assertEquals(PlayVoiceMail.class, op.getClass());
    }

    public void testCreateDeleteOperationFromServletPath() {
        MailboxOperation op = MailboxOperation.createMailboxOperationFromServletPath("/userId/folderId/foo/delete");
        assertEquals(MoveVoiceMail.class, op.getClass());
    }

    public void testCreateBadOperationFromServletPath() {
        try {
            MailboxOperation.createMailboxOperationFromServletPath("/userId/folderId/foo/unknown");
            fail("expected no operation");
        } catch (MalformedMailboxUrlException expected) {
            assertTrue(true);
        }
    }
}
