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

import org.apache.tapestry.AbstractComponent;
import org.apache.tapestry.AbstractPage;
import org.apache.tapestry.IMessages;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.PageRedirectException;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;

/**
 * Utility method for tapestry pages and components
 * 
 * DEVELOPERS NOTE: Static utility methods have their place but consider adding
 * methods to TapestryContext to avoid too many static methods and the limitations they incur
 */
public final class TapestryUtils {
    /**
     * Standard name for form validation delegate
     */
    public static final String VALIDATOR = "validator";

    /**
     * restrict construction
     */
    private TapestryUtils() {
        // intentionally empty
    }
    
    /**
     * Utility method to provide more descriptive unchecked exceptions for unmarshalling object
     * from Tapestry service Parameters.
     * 
     * @throws IllegalArgumentException if parameter is not there is wrong class type
     */
    public static final Object assertParameter(Class expectedClass, Object[] params, int index) {
        if (params == null || params.length < index) {
            throw new IllegalArgumentException("Missing parameter at position " + index);
        }

        if (params[index] != null) {
            Class actualClass = params[index].getClass();
            if (!expectedClass.isAssignableFrom(actualClass)) {
                throw new IllegalArgumentException("Object of class type "
                        + expectedClass.getName() + " was expected at position " + index
                        + " but class type was " + actualClass.getName());
            }
        }

        return params[index];
    }

    /**
     * Specialized version of assertParameter. Extracts beanId (Integer) from the first (index 0)
     * parameter of the cycle service
     * 
     * @param cycle current request cycle
     * @return bean id - exception is thrown if no id found
     */
    public static Integer getBeanId(IRequestCycle cycle) {
        return (Integer) assertParameter(Integer.class, cycle.getServiceParameters(), 0);
    }

    /**
     * Helper method to display standard "nice" stale link message
     * 
     * @param page page on which stale link is discovered
     * @param validatorName name of the validator delegate bean used to record validation errors
     */
    public static void staleLinkDetected(AbstractPage page, String validatorName) {
        IValidationDelegate validator = (IValidationDelegate) page.getBeans().getBean(
                validatorName);
        validator.setFormComponent(null);
        validator.record(new ValidatorException("The page is out of date. Please refresh."));
        throw new PageRedirectException(page);
    }

    /**
     * Helper method to display standard "nice" stale link message. Use only if standard
     * "validator" name has been used.
     * 
     * @param page page on which stale link is discovered
     */
    public static void staleLinkDetected(AbstractPage page) {
        staleLinkDetected(page, VALIDATOR);
    }

    /**
     * Check if there are any validation errors on the page. Use only if standard "validator" name
     * has been used.
     * 
     * Please note: this will only work properly if called after all components had a chance to
     * append register validation errors. Do not use in submit listeners other than form submit
     * listener.
     * 
     * @param page
     * @return true if no errors found
     */
    public static boolean isValid(AbstractPage page) {
        IValidationDelegate validator = getValidator(page);
        return !validator.getHasErrors();
    }

    /**
     * Retrieves the validator for the current page. Use only if standard "validator" name has
     * been used.
     * 
     * Use to record errors not related to any specific component.
     * 
     * @param page
     * @return validation delegate component
     */
    public static IValidationDelegate getValidator(AbstractComponent page) {
        IValidationDelegate validator = (IValidationDelegate) page.getBeans().getBean(VALIDATOR);
        return validator;
    }

    /**
     * Convience method that will quietly localize and act graceful if resources
     * are not set or found.
     */
    public static final String localize(IMessages messages, String key) {
        return localize(messages, null, key);        
    }

    /**
     * @param prefix prepended to key w/o '.' (e.g. fullkey = prefix + key)
     */
    public static final String localize(IMessages messages, String prefix, String key) {
        if (key == null || messages == null) {
            return key;
        }
        
        String fullKey = prefix != null ? prefix + key : key;
        String label = messages.getMessage(fullKey, key);
        
        return label;
    }    
}
