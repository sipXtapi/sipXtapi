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
package org.sipfoundry.sipxconfig.site;

import org.apache.tapestry.form.IPropertySelectionModel;

/**
 * LenSelectionModel
 */
public class LenSelectionModel implements IPropertySelectionModel {
    private static final int[] LENS = {3, 4, 5, 6 }; 

    public int getOptionCount() {
        return LENS.length;
    }

    public Object getOption(int index) {
        return new Integer(LENS[index]);
    }

    public String getLabel(int index) {
        return "" + LENS[index] + " digits"; 
    }

    public String getValue(int index) {
        return "" + LENS[index] + " digits"; 
    }

    public Object translateValue(String value) {
        int index;

        
        index = Integer.parseInt(value.substring(0,1));

        return new Integer( index );
    }
}
