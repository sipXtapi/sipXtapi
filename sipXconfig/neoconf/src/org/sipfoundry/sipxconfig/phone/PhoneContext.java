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

import java.io.Serializable;
import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.common.DataObjectSource;
import org.sipfoundry.sipxconfig.device.DeviceDefaults;
import org.sipfoundry.sipxconfig.setting.Group;

/**
 * Context for entire sipXconfig framework. Holder for service layer bean factories.
 */
public interface PhoneContext extends DataObjectSource {

    public static final String CONTEXT_BEAN_NAME = "phoneContext";

    public List getAvailablePhoneModels();

    /**
     * Commits the transaction and performs a batch of SQL commands to database. Call this as high
     * in the application stack as possible for better performance and data integrity.
     * 
     * You need to call this before you attempt to delete an object that was created before last
     * call to flush. Not unreasonable, most times you don't delete and object before it's
     * created, but happens a lot in unittests.
     */
    public void flush();

    public int getPhonesCount();

    public int getPhonesInGroupCount(Integer groupId);

    public List loadPhonesByPage(Integer groupId, int page, int pageSize, String[] orderBy,
            boolean orderAscending);

    public Collection loadPhones();

    public Collection getAllPhoneIds();
    
    public void storeLine(Line line);

    public void deleteLine(Line line);

    public Line loadLine(Integer id);

    public Phone newPhone(PhoneModel model);

    public Phone loadPhone(Integer id);

    public Integer getPhoneIdBySerialNumber(String serialNumber);

    public Object load(Class c, Serializable id);

    public void storePhone(Phone phone);

    public void deletePhone(Phone phone);

    public List getGroups();

    /**
     * Retrieves phone group by name.
     * 
     * @param phoneGroupName name of the group
     * @param createIfNotFound if true a new group with this name will be created, if false null
     *        is returned if group with a phoneGroupName is not found
     * @return phone group or null if group not found and createIfNotFound is false
     */
    public Group getGroupByName(String phoneGroupName, boolean createIfNotFound);

    /** unittesting only */
    public void clear();

    public String getSystemDirectory();

    public DeviceDefaults getPhoneDefaults();

    public Collection getPhonesByGroupId(Integer groupId);

    public Collection getPhonesByUserId(Integer userId);

    public void addToGroup(Integer groupId, Collection ids);

    public void removeFromGroup(Integer groupId, Collection ids);

    public void addUsersToPhone(Integer phoneId, Collection ids);
}
