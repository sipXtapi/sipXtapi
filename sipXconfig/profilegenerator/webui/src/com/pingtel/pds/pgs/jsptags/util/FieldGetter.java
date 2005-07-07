/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $$
 */
package com.pingtel.pds.pgs.jsptags.util;

public interface FieldGetter {
    
    public void setObject(Object o)
        throws IllegalArgumentException;
    
    public Object getField(String fieldName)
        throws IllegalAccessException;
}
