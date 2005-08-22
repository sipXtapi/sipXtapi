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
package org.sipfoundry.sipxconfig.common;


public abstract class UserDeleteListener implements DaoEventListener {

    public void onDelete(Object entity) {
        if (entity instanceof User) {
            onUserDelete((User) entity);
        }
    }

    public void onSave(Object entity_) {
    }

    protected abstract void onUserDelete(User user);
}
