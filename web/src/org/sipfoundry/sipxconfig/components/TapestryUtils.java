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

import org.apache.tapestry.PageRedirectException;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.valid.ValidationDelegate;
import org.apache.tapestry.valid.ValidatorException;

/**
 * Utility method for tapestry pages and components
 */
public final class TapestryUtils {

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
     * Helper method to display standard "nice" stale link message
     * 
     * @param page page on which stale link is discovered
     * @param validatorName name of the validator delegate bean used to record validation errors
     */
    public static void staleLinkDetected(BasePage page, String validatorName) {
        ValidationDelegate validator = (ValidationDelegate) page.getBeans()
                .getBean(validatorName);
        validator.setFormComponent(null);
        validator.record(new ValidatorException("The page is out of date. Please refresh."));
        throw new PageRedirectException(page);
    }
}
