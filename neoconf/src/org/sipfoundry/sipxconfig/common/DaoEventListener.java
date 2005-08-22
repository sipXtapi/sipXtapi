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
package org.sipfoundry.sipxconfig.common;


/**
 * Any beans that implement this interface will be called when entities are saved
 * or deleted.  Dao's must declare DaoEventDispatcher (or any of it's subclasses)
 * as it's Spring-Hibernate interceptor for events to be sent
 */
public interface DaoEventListener {

    /**
     * Is called before the actual entity is deleted
     */
    public void onDelete(Object entity);

    /**
     * Is called before the actual entity is saved or updated
     */
    public void onSave(Object entity);
}    
