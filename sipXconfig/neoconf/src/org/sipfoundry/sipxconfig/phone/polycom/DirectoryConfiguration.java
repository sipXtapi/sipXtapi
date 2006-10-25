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
package org.sipfoundry.sipxconfig.phone.polycom;

import java.util.Collection;

import org.apache.commons.collections.CollectionUtils;
import org.apache.commons.collections.Transformer;
import org.apache.velocity.VelocityContext;
import org.sipfoundry.sipxconfig.device.VelocityProfileGenerator;
import org.sipfoundry.sipxconfig.phonebook.Phonebook;
import org.sipfoundry.sipxconfig.phonebook.PhonebookEntry;
import org.sipfoundry.sipxconfig.phonebook.PhonebookManager;
import org.sipfoundry.sipxconfig.setting.BeanWithSettings;

public class DirectoryConfiguration extends VelocityProfileGenerator {
    private PhonebookManager m_phonebookManager;
    private Phonebook m_phonebook;

    public DirectoryConfiguration(BeanWithSettings phone, PhonebookManager phonebookManager,
            Phonebook phonebook) {
        super(phone);
        m_phonebook = phonebook;
        m_phonebookManager = phonebookManager;
    }

    @Override
    protected void addContext(VelocityContext context) {
        super.addContext(context);
    }

    public Collection<PolycomPhonebookEntry> getRows() {
        Collection<PhonebookEntry> phonebookEntries = m_phonebookManager.getRows(m_phonebook);
        return transformRows(phonebookEntries);
    }
    
    Collection<PolycomPhonebookEntry> transformRows(Collection<PhonebookEntry> phonebookEntries) {
        Transformer toPolycomEntries = new PolycomPhonebookEntryAdapter();
        Collection<PolycomPhonebookEntry> polycomEntries = (Collection<PolycomPhonebookEntry>) CollectionUtils
                .collect(phonebookEntries, toPolycomEntries);
        return polycomEntries;        
    }

    static class PolycomPhonebookEntryAdapter implements Transformer {
        public Object transform(Object input) {
            return new PolycomPhonebookEntry((PhonebookEntry) input);
        }
    }

    public static class PolycomPhonebookEntry {
        private PhonebookEntry m_entry;

        public PolycomPhonebookEntry(PhonebookEntry entry) {
            m_entry = entry;
        }

        /**
         * @return 
         */
        public String getFirstName() {
            // username if first and last name are null.  Otherwise it creates a
            // contact entry with no display label which is useless on polycom.
            String firstName = m_entry.getFirstName();
            if (firstName == null && m_entry.getLastName() == null) {
                return m_entry.getNumber();
            }
            return firstName;
        }

        public String getLastName() {
            return m_entry.getLastName();
        }

        public String getContact() {
            return m_entry.getNumber();
        }
    }
}
