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
     * If a row (or all rows) aren't selectable, you can use this 
     */
    public static final RowInfo UNSELECTABLE = new RowInfo<Object>() {
        
        public Object getSelectId(Object row) {
            return null;
        }

        public boolean isSelectable(Object row) {
            return false;
        }
        
    };
    
    /**
     * @return true if checkbox for row selection is to be rendered, false otherwise
     */
    boolean isSelectable(T row);
    
    /**
     * Provide custom primary key conversions.  primary for table selection
     */
    Object getSelectId(T row);
}
