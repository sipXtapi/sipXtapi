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

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.setting.Folder;



/**
 * Context for entire sipXconfig framework. Holder for service layer bean factories.
 */
public interface PhoneContext extends CoreContext {
    
    public static final int ROOT_PHONE_META_ID = 1000;
    
    public static final int ROOT_LINE_META_ID = 2000;
    
    public static final String CONTEXT_BEAN_NAME = "phoneContext";
    
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
     *          prviate int id = PhoneDao.UNSAVED_ID
     *         
     *           ...
     * </pre>
     */
    public static final int UNSAVED_ID = -1;
    
    public Phone getPhone(Endpoint endpoint);
    
    public Phone getPhone(int endpointId);
    
    public List getPhoneIds();

    public void setPhoneIds(List phoneIds);

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
    
    public List loadPhoneSummaries();      

    public void storeCredential(Credential credential);
    
    public void deleteCredential(Credential credential);

    public Credential loadCredential(int id);

    public void storeLine(Line line);
    
    public void deleteLine(Line line);

    public Line loadLine(int id);

    public Endpoint loadEndpoint(int id);
    
    public Object load(Class c, int id);
        
    public void storeEndpoint(Endpoint endpoint);

    public void deleteEndpoint(Endpoint endpoint);
    
    public Folder getRootPhoneFolder();

    public Folder getRootLineFolder();
}
