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
package org.sipfoundry.sipxconfig.phone;

/**
 * Most objects save save to database, and possible other types as well, can 
 * have an id that uniquely maps to them.  This captures that id for enviroments
 * that can leverage that, for example web interface that wants to draw checkboxes
 * for objects id selection.
 */
public interface PrimaryKeySource {
    
    public Object getPrimaryKey(); 

}
