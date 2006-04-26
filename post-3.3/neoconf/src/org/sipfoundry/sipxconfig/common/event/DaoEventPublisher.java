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
package org.sipfoundry.sipxconfig.common.event;

public interface DaoEventPublisher {
    /**
     * Call this to notify listeners that entity is about to be deleted
     */
    public void publishDelete(Object entity);

    /**
     * Call this to notify listeners that entity is about to be saved or updated
     */
    public void publishSave(Object entity);

}
