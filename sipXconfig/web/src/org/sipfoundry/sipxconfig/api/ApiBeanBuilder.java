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
package org.sipfoundry.sipxconfig.api;

/**
 * Transforming beans used in API to beans used in neoconf library 
 */
public interface ApiBeanBuilder {
        
    public void toApi(Object apiObject, Object otherObject);
    
    public void fromApi(Object apiObject, Object otherObject);        
}
