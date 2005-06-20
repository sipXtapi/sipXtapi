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

/**
 * Database services for setting business objects
 */
public interface SettingDao {

    /** common name found in spring file */
    public static final String CONTEXT_NAME = "settingDao";

    public void storeTag(Tag meta);

    public Tag loadRootTag(String resource);

    public Tag loadTag(int tagId);

    public void storeValueStorage(ValueStorage storage);

    public ValueStorage loadValueStorage(int storageId);
}
