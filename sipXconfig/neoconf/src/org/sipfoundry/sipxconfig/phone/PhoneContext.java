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

import java.util.Collection;
import java.util.List;
import java.util.Map;

import org.sipfoundry.sipxconfig.common.DataObjectSource;
import org.sipfoundry.sipxconfig.setting.Group;


/**
 * Context for entire sipXconfig framework. Holder for service layer bean factories.
 */
public interface PhoneContext extends DataObjectSource {
    
    public static final String CONTEXT_BEAN_NAME = "phoneContext";
        
    /**
     * Generate profile on phones in background
     * 
     * @param phones collection of phone objects
     */
    public void generateProfilesAndRestart(Collection phones);
    
    /**
     * Restart phones in background
     * 
     * @param phones collection of phone objects
     */
    public void restart(Collection phones);    

    public Map getPhoneFactoryIds();

    /**
     * Commits the transaction and performs a batch of SQL commands
     * to database. Call this as high in the application stack as possible
     * for better performance and data integrity.
     * 
     * You need to call this before you attempt to delete an object
     * that was created before last call to flush. Not unreasonable, most
     * times you don't delete and object before it's created, but happens
     * a lot in unittests. 
     */
    public void flush();
    
    public Collection loadPhones();      

    public void storeLine(Line line);
    
    public void deleteLine(Line line);

    public Line newLine(String factoryId);

    public Line loadLine(Integer id);
    
    public Phone newPhone(String factoryId);

    public Phone loadPhone(Integer id);
    
    public Object load(Class c, Integer id);
        
    public void storePhone(Phone phone);

    public void deletePhone(Phone phone);
    
    public Group loadRootGroup();
    
    public List getGroups();
    
    public List getGroupsWithoutRoot();
    
    public JobRecord loadJob(Integer id);
    
    public void storeJob(JobRecord job);

    /** unittesting only */
    public void clear();

    public String getSystemDirectory();

    public String getDnsDomain();

    public void deleteLinesForUser(Integer userId);
    
    /**
     * Get the number of members in each group
     * 
     * @return map {groupId as Integer, count as Integer} 
     */
    public Map getGroupMemberCountIndexedByGroupId();
}
