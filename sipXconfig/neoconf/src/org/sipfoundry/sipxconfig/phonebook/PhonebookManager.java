/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phonebook;

import java.util.Collection;

import org.sipfoundry.sipxconfig.common.User;


public interface PhonebookManager  {
    public static final String CONTEXT_BEAN_NAME = "phonebookManager";
    
    public Collection<Phonebook> getPhonebooks();

    public Phonebook getPhonebook(Integer phonebookId);
    
    public void savePhonebook(Phonebook phonebook);

    public String getExternalUsersDirectory();
    
    public Collection<PhonebookEntry> getRows(Phonebook phonebook);

    public Collection<PhonebookEntry> getRows(User consumer);
    
}
