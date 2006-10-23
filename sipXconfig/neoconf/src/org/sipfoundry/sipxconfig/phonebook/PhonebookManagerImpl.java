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

import java.io.File;
import java.io.IOException;
import java.util.Collection;

import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;

public class PhonebookManagerImpl extends SipxHibernateDaoSupport<Phonebook> implements PhonebookManager {
    private String m_externalUsersDirectory;

    public Phonebook getGlobalPhonebook() {
        Collection books = getHibernateTemplate().loadAll(Phonebook.class);
        Phonebook phonebook = (Phonebook) DaoUtils.requireOneOrZero(books, Phonebook.class.getName());
        return phonebook;
    }
    
    public Phonebook getPhonebook(Integer id) {
        Phonebook phonebook = load(Phonebook.class, id);
        return phonebook;
    }

    public void savePhonebook(Phonebook phonebook) {
        getHibernateTemplate().saveOrUpdate(phonebook);
    }

    /**
     * Where external user lists are kept. Callers can assume directory exists already 
     */
    public String getExternalUsersDirectory() {
        // HACK: No good place in unit test infrastructure to ensure phonebook directory exists
        // however unit test should be retooled when spring 2.0 gets updated
        return mkdirs(m_externalUsersDirectory);
    }
    
    static String mkdirs(String dir) {
        File d = new File(dir);
        if (!d.exists()) {
            if (!d.mkdirs()) {
                throw new RuntimeException(new IOException("Could not create directory " + d.getAbsolutePath()));
            }
        }
        return dir;
    }    

    public void setExternalUsersDirectory(String externalUsersDirectory) {
        m_externalUsersDirectory = externalUsersDirectory;
    }
}
