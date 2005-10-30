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
package org.sipfoundry.sipxconfig.components;

import java.util.Locale;

import junit.framework.TestCase;

import org.apache.tapestry.ApplicationRuntimeException;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.valid.IValidationDelegate;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.common.UserException;

public class TapestryContextTest extends TestCase {
    private TapestryContext m_context;

    static {
        // Note: this function is called by Tapestry object when first
        // ApplicationRunctimeException is called
        // for some reason it's really slow (couple of seconds)
        // I am calling it here explicitely, not to make it faster, but to better expose the reason
        // why the test is slow
        Locale.getAvailableLocales();
    }

    protected void setUp() throws Exception {
        m_context = new TapestryContext();
    }

    public void testTreatUserExceptionAsValidationError() {
        MockControl actionControl = MockControl.createControl(IActionListener.class);
        IActionListener action = (IActionListener) actionControl.getMock();
        action.actionTriggered(null, null);
        actionControl.replay();

        MockControl validatorControl = MockControl.createControl(IValidationDelegate.class);
        IValidationDelegate validator = (IValidationDelegate) validatorControl.getMock();
        validatorControl.replay();

        IActionListener listener = m_context.treatUserExceptionAsValidationError(validator,
                action);
        listener.actionTriggered(null, null);

        actionControl.verify();
        validatorControl.verify();
    }

    public void testTreatUserExceptionAsValidationErrorUserException() {
        Throwable exception = new UserException("kuku") {};

        MockControl actionControl = MockControl.createControl(IActionListener.class);
        IActionListener action = (IActionListener) actionControl.getMock();
        action.actionTriggered(null, null);
        actionControl.setThrowable(new ApplicationRuntimeException(exception));
        actionControl.replay();

        MockControl validatorControl = MockControl.createControl(IValidationDelegate.class);
        validatorControl.setDefaultMatcher(MockControl.ALWAYS_MATCHER);
        IValidationDelegate validator = (IValidationDelegate) validatorControl.getMock();
        validator.record(null);
        validatorControl.replay();

        IActionListener listener = m_context.treatUserExceptionAsValidationError(validator,
                action);
        listener.actionTriggered(null, null);

        actionControl.verify();
        validatorControl.verify();
    }

    public void testTreatUserExceptionAsValidationErrorOtherException() {
        Throwable exception = new NullPointerException();

        MockControl actionControl = MockControl.createControl(IActionListener.class);
        IActionListener action = (IActionListener) actionControl.getMock();
        action.actionTriggered(null, null);
        actionControl.setThrowable(new ApplicationRuntimeException(exception));
        actionControl.replay();

        MockControl validatorControl = MockControl.createControl(IValidationDelegate.class);
        IValidationDelegate validator = (IValidationDelegate) validatorControl.getMock();
        validatorControl.replay();

        IActionListener listener = m_context.treatUserExceptionAsValidationError(validator,
                action);
        try {
            listener.actionTriggered(null, null);
            fail("ApplicationRuntimeException expected");
        } catch (ApplicationRuntimeException are) {
            assertTrue(are.getCause() instanceof NullPointerException);
        }

        actionControl.verify();
        validatorControl.verify();
    }

    public void testTreatUserExceptionAsValidationErrorNull() {
        MockControl actionControl = MockControl.createControl(IActionListener.class);
        IActionListener action = (IActionListener) actionControl.getMock();
        action.actionTriggered(null, null);
        actionControl.setThrowable(new ApplicationRuntimeException("kuku"));
        actionControl.replay();

        MockControl validatorControl = MockControl.createControl(IValidationDelegate.class);
        IValidationDelegate validator = (IValidationDelegate) validatorControl.getMock();
        validatorControl.replay();

        IActionListener listener = m_context.treatUserExceptionAsValidationError(validator,
                action);
        try {
            listener.actionTriggered(null, null);
            fail("ApplicationRuntimeException expected");
        } catch (ApplicationRuntimeException are) {
            assertNull(are.getCause());
        }

        actionControl.verify();
        validatorControl.verify();
    }

}
