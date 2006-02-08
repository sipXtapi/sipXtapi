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
package org.sipfoundry.sipxconfig.site.admin.commserver;

import java.util.List;

import junit.framework.TestCase;

import org.apache.tapestry.test.Creator;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Command;
import org.sipfoundry.sipxconfig.admin.commserver.SipxProcessContext.Process;

public class RestartReminderPanelTest extends TestCase {
    private final static Process TEST_PROC[] = {
        Process.AUTH_PROXY, Process.PRESENCE_SERVER
    };

    private Creator m_pageMaker = new Creator();
    private RestartReminderPanel m_restartReminder;

    protected void setUp() throws Exception {
        m_restartReminder = (RestartReminderPanel) m_pageMaker
                .newInstance(RestartReminderPanel.class);
    }

    public void testGetProcessesToRestartLater() {
        m_restartReminder.setRestartLater(true);

        m_restartReminder.setProcesses(null);
        assertNull(m_restartReminder.getProcessesToRestart());

        m_restartReminder.setProcesses(TEST_PROC);
        assertNull(m_restartReminder.getProcessesToRestart());
    }

    public void testGetProcessesToRestartNow() {
        m_restartReminder.setRestartLater(false);

        m_restartReminder.setProcesses(null);
        List processesToRestart = m_restartReminder.getProcessesToRestart();
        assertEquals(Process.getRestartable().size(), processesToRestart.size());

        m_restartReminder.setProcesses(TEST_PROC);
        processesToRestart = m_restartReminder.getProcessesToRestart();
        assertEquals(TEST_PROC.length, processesToRestart.size());
        for (int i = 0; i < TEST_PROC.length; i++) {
            assertEquals(TEST_PROC[i], processesToRestart.get(i));
        }
    }

    public void testRestartLater() throws Exception {
        MockControl contextCtrl = MockControl.createControl(SipxProcessContext.class);
        SipxProcessContext context = (SipxProcessContext) contextCtrl.getMock();
        contextCtrl.replay();

        m_restartReminder.setRestartLater(true);
        m_restartReminder.setSipxProcessContext(context);

        m_restartReminder.restart();

        contextCtrl.verify();
    }

    public void testRestartNow() throws Exception {
        MockControl contextCtrl = MockControl.createControl(SipxProcessContext.class);
        SipxProcessContext context = (SipxProcessContext) contextCtrl.getMock();
        context.manageServices(Process.getRestartable(), Command.RESTART);
        contextCtrl.replay();

        m_restartReminder.setRestartLater(false);
        m_restartReminder.setSipxProcessContext(context);

        m_restartReminder.restart();

        contextCtrl.verify();
    }
}
