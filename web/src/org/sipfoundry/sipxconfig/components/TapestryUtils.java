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


/**
 * Utility method for tapestry pages and components
 */
public final class TapestryUtils {
    
    /**
     * restrict construction 
     */
    private TapestryUtils() {
    }
    
    /**
     * Utility method to provide more descriptive unchecked exceptions for unmarshalling
     * object from Tapestry service Parameters.
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
                throw new IllegalArgumentException("Object of class type " + expectedClass.getName() 
                        + " was expected at position " + index + " but class type was " 
                        + actualClass.getName());
            }
        }
        
        return params[index];
    }
}
