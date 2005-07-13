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

import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.common.DataObjectSource;

/**
 * Database services for setting business objects
 */
public interface SettingDao extends DataObjectSource {

    /** common name found in spring file */
    public static final String CONTEXT_NAME = "settingDao";

    public void storeGroup(Group group);
    
    public void deleteGroup(Group group);
    
    public List getGroups(String resource);
    
    public List getGroupsWithoutRoot(String source);

    public Group loadRootGroup(String resource);

    public Group loadGroup(Integer tagId);

    public void storeValueStorage(ValueStorage storage);

    public ValueStorage loadValueStorage(Integer storageId);
    
    /**
     * Get the number of members in each group
     * 
     * @return map {groupId as Integer, count as Integer} 
     */
    public Map getGroupMemberCountIndexedByGroupId(Class groupOwner);
}
