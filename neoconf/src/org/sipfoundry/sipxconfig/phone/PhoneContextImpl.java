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

import org.sipfoundry.sipxconfig.setting.Folder;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;
import org.springframework.orm.hibernate.support.HibernateDaoSupport;

/**
 * Context for entire sipXconfig framework. Holder for service layer bean factories.
 */
public class PhoneContextImpl extends HibernateDaoSupport implements BeanFactoryAware,
        PhoneContext {
    
    private SettingDao m_settingDao;

    private BeanFactory m_beanFactory;

    private List m_phoneIds;

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
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

    public Phone getPhone(Integer endpointId) {
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

    public Credential loadCredential(Integer id) {
        return (Credential) getHibernateTemplate().load(Credential.class, id);
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

    public Line loadLine(Integer id) {
        return (Line) getHibernateTemplate().load(Line.class, id);
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

    public Endpoint loadEndpoint(Integer id) {
        return (Endpoint) getHibernateTemplate().load(Endpoint.class, id);
    }

    public Object load(Class c, Integer id) {
        return getHibernateTemplate().load(c, id);
    }
    
    public Folder loadRootEndpointFolder() {
       return m_settingDao.loadRootFolder(Endpoint.FOLDER_RESOURCE_NAME);
    }
    
    public Folder loadRootLineFolder() {
        return m_settingDao.loadRootFolder(Line.FOLDER_RESOURCE_NAME);
    }    

    /** unittesting only */
    public void clear() {
        getHibernateTemplate().delete("from Line");
        getHibernateTemplate().delete("from Endpoint");
        getHibernateTemplate().delete("from Folder");
        getHibernateTemplate().delete("from ValueStorage");
    }
}
