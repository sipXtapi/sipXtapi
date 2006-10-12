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
package org.sipfoundry.sipxconfig.site.phonebook;

import java.io.File;

import org.sipfoundry.sipxconfig.components.PageWithCallback;

public abstract class EditPhonebook extends PageWithCallback {
    
    public void savePhonebook() {
        // TODO
    }
    
    public File getPhonebookFile() {
        return new File("importUsers.csv");
    }
    
    public void setPhonebookFile(File phonebook) {
        // TODO
    }
    
    public File getPhonebookDirectory() {
        return new File(".");
    }

}
