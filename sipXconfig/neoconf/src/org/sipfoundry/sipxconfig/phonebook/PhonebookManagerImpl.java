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

import org.sipfoundry.sipxconfig.common.DaoUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;

public class PhonebookManagerImpl extends SipxHibernateDaoSupport<Phonebook> implements PhonebookManager {

    public Phonebook getPhonebook() {
        Collection books = getHibernateTemplate().loadAll(Phonebook.class);
        Phonebook phonebook = (Phonebook) DaoUtils.requireOneOrZero(books, Phonebook.class.getName());
        return phonebook;
    }

    public void savePhonebook(Phonebook phonebook) {
        getHibernateTemplate().saveOrUpdate(phonebook);
    }
}
