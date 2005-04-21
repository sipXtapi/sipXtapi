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
package org.sipfoundry.sipxconfig.site.admin;

import java.util.Collection;

import org.apache.tapestry.BaseComponent;

public abstract class UserRingTable extends BaseComponent {
    public abstract Collection getRowsToDelete();
    
    public abstract Collection getRowsToMoveUp();

    public abstract Collection getRowsToMoveDown();
    
    public abstract boolean getAddRow();
}
