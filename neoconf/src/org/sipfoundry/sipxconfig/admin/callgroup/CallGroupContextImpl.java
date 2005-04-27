/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.admin.callgroup;

import java.util.ArrayList;
import java.util.Collection;
import java.util.Iterator;
import java.util.List;

import org.sipfoundry.sipxconfig.admin.forwarding.GenerateMessage;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.Organization;
import org.sipfoundry.sipxconfig.phone.PhoneContext;
import org.springframework.jms.core.JmsOperations;
import org.springframework.orm.hibernate.HibernateTemplate;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

/**
 * Hibernate implementation of the call group context
 */
public class CallGroupContextImpl extends HibernateDaoSupport implements CallGroupContext {
    private PhoneContext m_phoneContext;
    private CoreContext m_coreContext;

    private JmsOperations m_jms;

    public CallGroup loadCallGroup(Integer id) {
        return (CallGroup) getHibernateTemplate().load(CallGroup.class, id);
    }

    public void storeCallGroup(CallGroup callGroup) {
        getHibernateTemplate().saveOrUpdate(callGroup);
    }

    public void removeCallGroups(Collection ids) {
        HibernateTemplate template = getHibernateTemplate();
        Collection entities = new ArrayList(ids.size());
        for (Iterator i = ids.iterator(); i.hasNext();) {
            Integer id = (Integer) i.next();
            Object entity = template.load(CallGroup.class, id);
            entities.add(entity);
        }
        template.deleteAll(entities);
    }

    public List getCallGroups() {
        return getHibernateTemplate().loadAll(CallGroup.class);
    }

    public void duplicateCallGroups(Collection ids_) {
        // TODO Auto-generated method stub
    }

    /**
     * Remove all call groups - mostly used for testing
     */
    public void clear() {
        HibernateTemplate template = getHibernateTemplate();
        Collection all = template.loadAll(CallGroup.class);
        template.deleteAll(all);
    }

    public void activateCallGroups() {
        List callGroups = getCallGroups();

        for (Iterator i = callGroups.iterator(); i.hasNext();) {
            CallGroup cg = (CallGroup) i.next();
            cg.activate(m_phoneContext);
        }

        // make profilegenerator to propagate new aliases
        if (null != m_jms) {
            m_jms.send(new GenerateMessage(GenerateMessage.TYPE_ALIAS));
        }
    }

    /**
     * Generate aliases for all call groups
     */
    public List getCallGroupAliases() {
        List callGroups = getCallGroups();
        List allAliases = new ArrayList();
        Organization org = m_coreContext.loadRootOrganization();
        for (Iterator i = callGroups.iterator(); i.hasNext();) {
            CallGroup cg = (CallGroup) i.next();
            allAliases.addAll(cg.generateAliases(org.getDnsDomain()));
        }
        return allAliases;
    }

    public void setJms(JmsOperations jms) {
        m_jms = jms;
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public void setPhoneContext(PhoneContext phoneContext) {
        m_phoneContext = phoneContext;
    }
}
