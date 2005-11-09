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

import java.util.Set;

/**
 * Transforming beans used in API to beans used in other systems 
 */
public interface ApiBeanBuilder {
        
    public void toApiObject(Object apiObject, Object myObject, Set properties);
    
    public void toMyObject(Object myObject, Object apiObject, Set properties);
}
