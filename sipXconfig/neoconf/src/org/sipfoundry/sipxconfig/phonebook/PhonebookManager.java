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

public interface PhonebookManager  {
    public static final String CONTEXT_BEAN_NAME = "phonebookManager";
    
    public Phonebook getPhonebook();
    
    public void savePhonebook(Phonebook phonebook);

}
