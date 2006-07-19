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
package org.sipfoundry.sipxconfig.admin.intercom;

import java.util.Collection;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import org.sipfoundry.sipxconfig.common.SipxCollectionUtils;
import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
import org.sipfoundry.sipxconfig.phone.Phone;
import org.sipfoundry.sipxconfig.setting.Group;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.orm.hibernate3.HibernateTemplate;

public class IntercomManagerImpl
    extends SipxHibernateDaoSupport
    implements IntercomManager, BeanFactoryAware {

    public static final String CONTEXT_BEAN_NAME = "intercomManagerImpl";
    
    private BeanFactory m_beanFactory;

    public Intercom newIntercom() {
        return (Intercom) m_beanFactory.getBean(Intercom.class.getName());
    }

    /**
     * Return an Intercom instance. Create one if none exist. Throw UserException
     * if more than one Intercom instance exists, since the caller is assuming that
     * there can be at most one Intercom instance.
     */
    public Intercom getIntercom() {
        List intercoms = loadIntercoms();
        int numIntercoms = getNumIntercoms();
        Intercom intercom = null;
        if (numIntercoms == 0) {
            intercom = newIntercom();
        } else if (numIntercoms > 1) {
            throw new UserException("Expecting at most 1 Intercom, but there are " + numIntercoms);
        } else {
            intercom = (Intercom) intercoms.get(0);
        }
        return intercom;
    }
    
    public void saveIntercom(Intercom intercom) {
        getHibernateTemplate().saveOrUpdate(intercom);
    }

    public List loadIntercoms() {
        return getHibernateTemplate().loadAll(Intercom.class);
    }
    
    public int getNumIntercoms() {
        return SipxCollectionUtils.safeSize(loadIntercoms());
    }
    
    /**
     * Remove all intercoms - mostly used for testing
     */
    public void clear() {
        HibernateTemplate template = getHibernateTemplate();
        Collection intercoms = template.loadAll(Intercom.class);
        template.deleteAll(intercoms);
    }
    
    /**
     * Return the intercom associated with a phone, through the groups the phone
     * belongs to, or null if there is no intercom for the phone.
     * There should be at most one intercom for any phone. If there is more than
     * one, then return the first intercom found.
     */
    // Use a naive linear search here. There are usually very few intercoms or
    // groups, so that should be fine.
    public Intercom getIntercomForPhone(Phone phone) {
        Intercom theIntercom = null;   // will hold result
        List intercoms = loadIntercoms();
        for (Iterator iter = intercoms.iterator(); iter.hasNext();) {
            Intercom intercom = (Intercom) iter.next();
            Set intercomGroups = intercom.getGroups();
            Set phoneGroups = phone.getGroups();
            for (Iterator i = intercomGroups.iterator(); i.hasNext();) {
                Group intercomGroup = (Group) i.next();
                if (phoneGroups.contains(intercomGroup)) {
                    theIntercom = intercom;
                    break;
                }
            }
            
        }
        return theIntercom;
    }

    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

}
