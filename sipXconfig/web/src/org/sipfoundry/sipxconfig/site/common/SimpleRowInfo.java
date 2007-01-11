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
package org.sipfoundry.sipxconfig.site.common;

import org.sipfoundry.sipxconfig.common.PrimaryKeySource;
import org.sipfoundry.sipxconfig.components.RowInfo;

/**
 * Default implementation of information about a row the Table component needs
 * to know to operate
 */
public class SimpleRowInfo implements RowInfo<PrimaryKeySource> {

    public Object getSelectId(PrimaryKeySource row) {
        return row.getPrimaryKey();
    }

    public boolean isSelectable(PrimaryKeySource row) {
        return true;
    }
}
