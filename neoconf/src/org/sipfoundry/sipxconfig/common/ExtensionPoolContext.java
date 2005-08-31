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
package org.sipfoundry.sipxconfig.common;

public interface ExtensionPoolContext extends DataObjectSource {
    public static final String CONTEXT_BEAN_NAME = "extensionPoolContext";
    
    public void saveExtensionPool(ExtensionPool pool);
    
    /**
     * Return the next free extension from the user extension pool,
     * or null if a free extension could not be found.
     */
    public Integer getNextFreeUserExtension();    
}
