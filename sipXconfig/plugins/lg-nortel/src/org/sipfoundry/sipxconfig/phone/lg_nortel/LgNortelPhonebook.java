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
package org.sipfoundry.sipxconfig.phone.lg_nortel;

import java.util.Collection;

import org.sipfoundry.sipxconfig.device.ProfileContext;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;

public class LgNortelPhonebook extends ProfileContext {
    
    private Collection<PhonebookEntry> m_phonebook;

    public LgNortelPhonebook(Collection<PhonebookEntry> phonebook) {
        super(null, "lg-nortel/mac-phonebook.csv.vm");
        m_phonebook = phonebook;
    }
    
    public Collection<PhonebookEntry> getPhonebook() {
        return m_phonebook;
    }
}
