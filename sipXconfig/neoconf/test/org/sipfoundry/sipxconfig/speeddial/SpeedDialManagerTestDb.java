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
package org.sipfoundry.sipxconfig.speeddial;

import org.sipfoundry.sipxconfig.SipxDatabaseTestCase;
import org.sipfoundry.sipxconfig.TestHelper;
import org.springframework.context.ApplicationContext;

public class SpeedDialManagerTestDb extends SipxDatabaseTestCase {

    private SpeedDialManager m_context;

    protected void setUp() throws Exception {
        ApplicationContext appContext = TestHelper.getApplicationContext();
        m_context = (SpeedDialManager) appContext.getBean(SpeedDialManager.CONTEXT_BEAN_NAME);
        TestHelper.cleanInsert("ClearDb.xml");
    }

    public void testGetSpeedDialForUser() throws Exception {
        TestHelper.insertFlat("speeddial/speeddial.db.xml");
        SpeedDial speedDial = m_context.getSpeedDialForUserId(1001);
        assertNotNull(speedDial);
        assertEquals(3, speedDial.getButtons().size());
        assertEquals("222", speedDial.getButtons().get(2).getNumber());
    }

    public void testGetNewSpeedDialForUser() throws Exception {
        TestHelper.insertFlat("speeddial/speeddial.db.xml");
        SpeedDial speedDial = m_context.getSpeedDialForUserId(1002);
        assertNotNull(speedDial);
        assertEquals(0, speedDial.getButtons().size());
    }

    public void testSaveSpeedDialForUser() throws Exception {
        TestHelper.insertFlat("speeddial/speeddial.db.xml");
        SpeedDial speedDial = m_context.getSpeedDialForUserId(1002);

        final int buttonCount = 5;
        for (int i = 0; i < buttonCount; i++) {
            Button b = new Button();
            b.setLabel("testSave");
            b.setNumber(String.valueOf(i));
            speedDial.getButtons().add(b);
        }

        m_context.saveSpeedDial(speedDial);
        assertEquals(buttonCount, getConnection().getRowCount("speeddial_button",
                "WHERE label = 'testSave'"));
    }
}
