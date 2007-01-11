/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.components;


/**
 * Used by sipX table component to influence rendering rows.
 */
public interface RowInfo<T> {
    /**
     * @return true if checkbox for row selection is to be rendered, false otherwise
     */
    boolean isSelectable(T row);
    
    /**
     * Provide custom primary key conversions.  primary for table selection
     */
    Object getSelectId(T row);
}
