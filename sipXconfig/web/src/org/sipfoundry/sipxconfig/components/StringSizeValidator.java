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

import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.IBinding;
import org.apache.tapestry.IComponent;
import org.apache.tapestry.IPage;
import org.apache.tapestry.form.IFormComponent;
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
 * The suggested way of using this class is to define bean in page spec. <code>
 *   <bean name="descriptionValidator" class="org.sipfoundry.sipxconfig.components.StringSizeValidator">
 *     <set-property name="component" expression="components.gatewayDescription"/>
 *     <set-property name="max" expression="20"/>
 *   </bean>
 * </code>
 * 
 * And then call it in formSubmit lsitener
 * 
 * <code>
 * IValidationDelegate delegate = (IValidationDelegate) getBeans().getBean("validator");
 * StringSizeValidator descriptionValidator = (StringSizeValidator) getBeans().getBean("descriptionValidator");
 * descriptionValidator.validate(delegate);
 * </code>
 * 
 */
public class StringSizeValidator {
    public static final int DEFAULT_MAX_LEN = 256;
    private static final String DISPLAY_NAME = "displayName";
    private static final String ERROR = "Enter at most {0} characters for {1}";

    private IFormComponent m_component;
    private int m_max = DEFAULT_MAX_LEN;
    private String m_labelSuffix = "Label";

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
        String labelName = m_component.getName() + m_labelSuffix;
        IPage page = m_component.getPage();
        IComponent label = (IComponent) page.getComponents().get(labelName);
        if (null != label) {
            return (String) label.getProperty(DISPLAY_NAME);
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