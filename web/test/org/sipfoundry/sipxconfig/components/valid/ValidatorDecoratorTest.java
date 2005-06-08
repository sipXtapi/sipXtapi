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
package org.sipfoundry.sipxconfig.components.valid;

import junit.framework.TestCase;

import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.valid.IValidator;
import org.easymock.MockControl;
import org.sipfoundry.sipxconfig.components.valid.ValidatorDecorator;

public class ValidatorDecoratorTest extends TestCase {

    protected void setUp() throws Exception {
        super.setUp();
    }

    public void testToObject() throws Exception {
        MockControl control = MockControl.createStrictControl(IValidator.class);
        control.setDefaultMatcher(MockControl.EQUALS_MATCHER);
        IValidator validator = (IValidator) control.getMock();
        validator.toObject(null, "a");
        control.setReturnValue("ab");
        control.replay();

        ValidatorDecoratorMock wrapper = new ValidatorDecoratorMock();
        wrapper.setDelegate(validator);
        assertEquals("abc", wrapper.toObject(null, ""));
        control.verify();
    }

    private static class ValidatorDecoratorMock extends ValidatorDecorator {
        protected String preValidate(IFormComponent field_, String input) {
            return input + "a";
        }

        protected Object postValidate(IFormComponent field_, Object input) {
            return input.toString() + "c";
        }
    }
}
