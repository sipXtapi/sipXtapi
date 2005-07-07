/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.components;

import org.apache.tapestry.ApplicationRuntimeException;
import org.apache.tapestry.IActionListener;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;
import org.sipfoundry.sipxconfig.common.UserException;

/**
 * Tapestry utilities available to web pages 
 */
public class TapestryContext {
    
    /**
     * Add a option to the dropdown model with a label to instruct the user to make a selection.
     * If not item is selected, your business object method will be explicitly set to null
     */
    public IPropertySelectionModel instructUserToSelect(IPropertySelectionModel model) {   
        ExtraOptionModelDecorator decorated = new ExtraOptionModelDecorator();
        decorated.setExtraLabel("select...");
        decorated.setExtraOption(null);
        decorated.setModel(model);
        
        return decorated;
    }
    
    /**
     * Translates UserExceptions into form errors instead redirecting to an error page.
     */
    public IActionListener treatUserExceptionAsValidationError(IValidationDelegate validator, 
            IActionListener listener) {
        return new UserExceptionAdapter(validator, listener);        
    }
    
    static class UserExceptionAdapter implements IActionListener {
        
        private IActionListener m_listener;
        
        private IValidationDelegate m_validator;
        
        UserExceptionAdapter(IValidationDelegate validator, IActionListener listener) {
            m_listener = listener;
            m_validator = validator;
        }

        public void actionTriggered(IComponent component, IRequestCycle cycle) {
            try {
                m_listener.actionTriggered(component, cycle);
            } catch (ApplicationRuntimeException are) {
                Throwable cause = are.getCause();
                if (cause instanceof UserException) {
                    m_validator.record(new ValidatorException(cause.getMessage()));                    
                } else {
                    throw are;
                }
            }
        }
    }
}
