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

import java.text.MessageFormat;
import java.util.Collection;
import java.util.Iterator;

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.IBinding;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.valid.FieldLabel;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;

/**
 * StringSizeValidator can be used for validating text area input.
 * 
 * Standard Tapestry validators at the moment only work with ValidField component. This validator
 * can be used with any component, however its validate function needs to called explicitly. This
 * is a half measure, but Tapestry 3.1 is supposedly rewriting validation support, so it probably
 * does not make sense to invest in a better validation framework.
 * 
 */
public class StringSizeValidator {

    /** should be less than or equals DB field length */
    public static final int DEFAULT_MAX_LEN = 255;

    private static final String ERROR = "Enter at most {0} characters for {1}";

    private IFormComponent m_component;
    private int m_max = DEFAULT_MAX_LEN;

    public void validate(IValidationDelegate delegate) {
        IBinding binding = m_component.getBinding("value");
        String property = binding.getString();
        if (null != property && property.length() > m_max) {
            delegate.setFormComponent(m_component);
            delegate.record(getError(), ValidationConstraint.TOO_LARGE);
        }
    }

    public void setComponent(IFormComponent component) {
        m_component = component;
    }

    public void setMax(int max) {
        m_max = max;
    }

    private String getDisplayName() {
        String displayName = m_component.getDisplayName();
        if (null != displayName) {
            return displayName;
        }

        // find the label for the component and return its display name
        IComponent container = m_component.getContainer();
        Collection components = container.getComponents().values();
        for (Iterator i = components.iterator(); i.hasNext();) {
            IComponent component = (IComponent) i.next();
            if (component instanceof FieldLabel) {
                FieldLabel label = (FieldLabel) component;
                if (label.getField() == m_component) {
                    return label.getDisplayName();
                }
            }
        }

        return StringUtils.EMPTY;
    }

    private String getError() {
        MessageFormat format = new MessageFormat(ERROR);
        return format.format(new Object[] {
            new Integer(m_max), getDisplayName()
        });
    }
}
