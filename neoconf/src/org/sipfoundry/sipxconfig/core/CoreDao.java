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
package org.sipfoundry.sipxconfig.core;

/**
 * Comments
 */
public interface CoreDao {

    /**
     * Load a logical phone record from the databae and build a logical phone object
     * 
     * @return
     */
    public Object findById(Class c, int id);

    /**
     * Load a logical phone record from the databae and build a logical phone object
     * 
     * @return
     * throws ObjectNotFoundException
     */
    public Object requireById(Class c, int id);

}
