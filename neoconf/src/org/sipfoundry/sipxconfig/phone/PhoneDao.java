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

import java.util.List;


/**
 * Create/Retrieve/Update/Delete for Object in phone
 * package that r/w to a database.
 */
public interface PhoneDao {

    /**
     * infinite value when specifing depth parameter to some functions 
     */
    public static final int CASCADE = -1;
    
    /** 
     * int value for objects that haven't been saved to database yet
     * <pre>
     *   Example:
     *      public class MyObject {
     * 
     *          prviate int id = UNSAVED_ID
     *         
     *           ...
     * </pre>
     */
    public static final int UNSAVED_ID = -1;
    
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
    
    public void storeSetting(Setting setting, int depth);

    public void storeSetting(Setting setting);
    
    public void deleteSetting(Setting setting);
    
    public SettingSet loadSettings(int id);

    public Organization loadRootOrganization();
    
    public User loadUser(int id);
    
    public User loadUserByDisplayId(String displayId);
    
    public Endpoint loadEndpoint(int id);
        
    public List loadPhoneSummaries(PhoneSummaryFactory table);
    
    /**
     * Complete with endpoint and users loaded
     * @return list of endpoint objects
    public List loadEndpoints();

    public List loadEndpointAssigments();
     */

    public void storeEndpoint(Endpoint endpoint);

    public void deleteEndpoint(Endpoint endpoint);
    
    public void storeEndpointAssignment(EndpointAssignment assignment);
    
    public EndpointAssignment loadEndpointAssignment(int assignmentId);

    public void deleteEndpointAssignment(EndpointAssignment assignment);
}
