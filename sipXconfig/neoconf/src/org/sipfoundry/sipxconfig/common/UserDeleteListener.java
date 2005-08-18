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

import java.io.Serializable;

import org.hibernate.type.Type;

public abstract class UserDeleteListener implements DaoEventListener {

    public void onDelete(Object entity, Serializable id_, Object[] state_,
            String[] propertyNames_, Type[] types_) {
        if (entity instanceof User) {
            onUserDelete((User) entity);
        }
    }

    public boolean onSave(Object entity_, Serializable id_, Object[] state_,
            String[] propertyNames_, Type[] types_) {
        return false;
    }

    protected abstract void onUserDelete(User user);
}
