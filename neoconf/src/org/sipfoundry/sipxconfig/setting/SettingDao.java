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
package org.sipfoundry.sipxconfig.setting;

import java.util.Collection;

import org.sipfoundry.sipxconfig.common.DataObjectSource;

/**
 * Database services for setting business objects
 */
public interface SettingDao extends DataObjectSource {

    /** common name found in spring file */
    public static final String CONTEXT_NAME = "settingDao";

    public void storeGroup(Group meta);
    
    public Collection getGroups(String resource);

    public Group loadRootGroup(String resource);

    public Group loadGroup(int tagId);

    public void storeValueStorage(ValueStorage storage);

    public ValueStorage loadValueStorage(int storageId);
}
