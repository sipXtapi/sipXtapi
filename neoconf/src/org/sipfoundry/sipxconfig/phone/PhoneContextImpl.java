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

import java.util.ArrayList;
import java.util.List;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

/**
 * Context for entire sipXconfig framework. Holder for service layer bean factories.
 */
public class PhoneContextImpl extends HibernateDaoSupport implements BeanFactoryAware,
        PhoneContext {
    private CoreContext m_coreContext;

    private BeanFactory m_beanFactory;

    private List m_phoneIds;

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    /**
     * Callback that supplies the owning factory to a bean instance.
     */
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public Phone getPhone(Endpoint endpoint) {
        return (Phone) m_beanFactory.getBean(endpoint.getPhoneId());
    }

    public Phone getPhone(int endpointId) {
        return getPhone(loadEndpoint(endpointId));
    }

    public List getPhoneIds() {
        return m_phoneIds;
    }

    public void setPhoneIds(List phoneIds) {
        m_phoneIds = phoneIds;
    }

    public void flush() {
        getHibernateTemplate().flush();
    }

    public void storeCredential(Credential credential) {
        getHibernateTemplate().saveOrUpdate(credential);
    }

    public void deleteCredential(Credential credential) {
        getHibernateTemplate().delete(credential);
    }

    public Credential loadCredential(int id) {
        return (Credential) getHibernateTemplate().load(Credential.class, new Integer(id));
    }

    public void storeEndpoint(Endpoint endpoint) {
        getHibernateTemplate().saveOrUpdate(endpoint);
    }

    public void deleteEndpoint(Endpoint endpoint) {
        getHibernateTemplate().delete(endpoint);
    }

    public void storeLine(Line line) {
        getHibernateTemplate().saveOrUpdate(line);
    }

    public void deleteLine(Line line) {
        getHibernateTemplate().delete(line);
    }

    public Line loadLine(int id) {
        return (Line) getHibernateTemplate().load(Line.class, new Integer(id));
    }

    public List loadPhoneSummaries() {
        String endpointQuery = "from Endpoint e left join fetch e.lines line left join fetch line.user";
        List endpoints = getHibernateTemplate().find(endpointQuery);
        List summaries = new ArrayList(endpoints.size());

        // TODO: Make this a cursor usable by tapestry by reconnecting back to
        // session
        for (int i = 0; i < endpoints.size(); i++) {
            Endpoint endpoint = (Endpoint) endpoints.get(i);
            PhoneSummary summary = new PhoneSummary();
            summary.setEndpoint(endpoint);
            summary.setPhone(getPhone(endpoint));
            summaries.add(summary);
        }

        return summaries;
    }

    public Endpoint loadEndpoint(int id) {
        return (Endpoint) getHibernateTemplate().load(Endpoint.class, new Integer(id));
    }

    public Object load(Class c, int id) {
        return getHibernateTemplate().load(c, new Integer(id));
    }
    
    // FIXME: the following functions are deprecated, use CoreContextDirectly
    public void saveUser(User user) {
        m_coreContext.saveUser(user);
    }

    public void deleteUser(User user) {
        m_coreContext.deleteUser(user);
    }

    public User loadUser(int id) {
        return m_coreContext.loadUser(id);
    }

    public User loadUserByDisplayId(String displayId) {
        return m_coreContext.loadUserByDisplayId(displayId);
    }

    public List loadUserByTemplateUser(User template) {
        return m_coreContext.loadUserByTemplateUser(template);
    }

    public Organization loadRootOrganization() {
        return m_coreContext.loadRootOrganization();
    }
}
