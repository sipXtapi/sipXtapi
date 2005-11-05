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

public interface ApiBeanBuilder {
        
    public void toApi(Object soapObject, Object otherObject);
    
    public void fromApi(Object soapObject, Object otherObject);        
}
