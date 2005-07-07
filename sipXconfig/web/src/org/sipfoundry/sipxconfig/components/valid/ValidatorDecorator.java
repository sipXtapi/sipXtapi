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

import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.valid.IValidator;
import org.apache.tapestry.valid.ValidatorException;

/**
 * Allows constructing decorators that modify user input before and after it has been validate
 */
public abstract class ValidatorDecorator implements IValidator {
    private IValidator m_delegate;

    public void setDelegate(IValidator delegate) {
        m_delegate = delegate;
    }
    
    public boolean isRequired() {
        return m_delegate.isRequired();
    }

    public String toString(IFormComponent field, Object value) {
        return m_delegate.toString(field, value);
    }

    public Object toObject(IFormComponent field, String input) throws ValidatorException {
        String modifiedInput = preValidate(field, input);
        Object value = m_delegate.toObject(field, modifiedInput);
        return postValidate(field, value);
    }

    public void renderValidatorContribution(IFormComponent field, IMarkupWriter writer,
            IRequestCycle cycle) {
        m_delegate.renderValidatorContribution(field, writer, cycle);
    }

    protected String preValidate(IFormComponent field_, String input) throws ValidatorException {
        return input;
    }

    protected Object postValidate(IFormComponent field_, Object input) throws ValidatorException {
        return input;
    }
}
