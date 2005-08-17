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
package org.sipfoundry.sipxconfig.site.user_portal;

import junit.framework.Test;
import net.sourceforge.jwebunit.WebTestCase;

import org.sipfoundry.sipxconfig.site.SiteTestHelper;
import org.sipfoundry.sipxconfig.site.TestPage;

/**
 * ChangePinTestUi
 */
public class ChangePinTestUi extends WebTestCase {
    private static final String CURRENT_PIN = TestPage.PIN;
    private static final String NEW_PIN = "5678";
    private static final String WRONG_PIN = "0000";
    private static final String NON_NUMERIC_PIN = "nerf";
    
    private static final String MSG_MUST_ENTER_VALUE = "You must enter a value";
    private static final String MSG_WRONG_PIN = "The current PIN that you entered is incorrect";
    private static final String MSG_NON_NUMERIC_PIN = "must be a numeric value";
    private static final String MSG_PIN_MISMATCH = "The new PIN and confirmed new PIN don't match";

    public static Test suite() throws Exception {
        return SiteTestHelper.webTestSuite(ChangePinTestUi.class);
    }

    protected void setUp() throws Exception {
        getTestContext().setBaseUrl(SiteTestHelper.getBaseUrl());
        SiteTestHelper.seedUser(getTester());
        SiteTestHelper.home(getTester());
        tester.clickLink("resetCoreContext");
        SiteTestHelper.home(getTester());
        clickLink("ChangePin");
    }

    
    public void testDisplayChangePin() throws Exception {
        assertTextNotPresent("An exception has occurred.");
        assertTextPresent("Change PIN");
    }
    
    public void testChangePin() throws Exception {
        // Change the PIN to a new value
        changePin(CURRENT_PIN, NEW_PIN);
        SiteTestHelper.assertNoUserError(tester);
        SiteTestHelper.assertNoException(tester);

        // Change the PIN back to its old value
        changePin(NEW_PIN, CURRENT_PIN);
        SiteTestHelper.assertNoUserError(tester);
        SiteTestHelper.assertNoException(tester);
    }
    
    public void testEmptyCurrentPin() throws Exception {
        changePin("", NEW_PIN);
        SiteTestHelper.assertNoException(tester);
        assertTextPresent(MSG_MUST_ENTER_VALUE);
    }
    
    public void testEmptyNewPin() throws Exception {
        changePin(CURRENT_PIN, "");
        SiteTestHelper.assertNoException(tester);
        assertTextPresent(MSG_MUST_ENTER_VALUE);
    }
    
    public void testWrongPin() throws Exception {
        changePin(WRONG_PIN, NEW_PIN);
        assertTextPresent(MSG_WRONG_PIN);
        SiteTestHelper.assertNoException(tester);
    }
    
    public void testNonNumericPin() throws Exception {
        changePin(CURRENT_PIN, NON_NUMERIC_PIN);
        assertTextPresent(MSG_NON_NUMERIC_PIN);
        SiteTestHelper.assertNoException(tester);
    }
   
    public void testNewPinMismatch() throws Exception {
        changePin(CURRENT_PIN, NEW_PIN, WRONG_PIN);
        assertTextPresent(MSG_PIN_MISMATCH);
        SiteTestHelper.assertNoException(tester);
    }
    
    private void changePin(String oldPin, String newPin, String newPinRepeated) {
        setFormElement("currentPin", oldPin);
        setFormElement("password", newPin);
        setFormElement("confirmPassword", newPinRepeated);
        clickButton("form:apply");        
    }
    
    private void changePin(String oldPin, String newPin) {
        changePin(oldPin, newPin, newPin);
    }
}
