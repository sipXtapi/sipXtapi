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

    public List getPhoneFactoryIds() {
        return m_phoneIds;
    }

    public void setPhoneFactoryIds(List phoneIds) {
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

    public void storePhone(Phone phone) {
        getHibernateTemplate().saveOrUpdate(phone.getPhoneMetaData());
    }

    public void deletePhone(Phone phone) {
        getHibernateTemplate().delete(phone.getPhoneMetaData());
    }

    public void storeLine(Line line) {
        getHibernateTemplate().saveOrUpdate(line.getLineMetaData());
    }

    public void deleteLine(Line line) {
        getHibernateTemplate().delete(line.getLineMetaData());
    }

    public Line loadLine(Integer id) {
        return loadLineFromFactory((LineMetaData) getHibernateTemplate().load(LineMetaData.class, id));
    }

    public List loadPhones() {
        String endpointQuery = "from PhoneMetaData p";
        List metas = getHibernateTemplate().find(endpointQuery);
        List phones = new ArrayList(metas.size());

        // TODO: Make this a cursor usable by tapestry by reconnecting back to
        // session
        for (int i = 0; i < metas.size(); i++) {
            PhoneMetaData meta = (PhoneMetaData) metas.get(i);
            phones.add(loadPhoneFromFactory(meta));
        }

        return phones;
    }

    public Phone loadPhone(Integer id) {
        return loadPhoneFromFactory((PhoneMetaData) getHibernateTemplate().load(PhoneMetaData.class, id));
    }
    
    public Phone newPhone(String factoryId) {
        Phone phone = (Phone) m_beanFactory.getBean(factoryId);
        phone.setPhoneMetaData(new PhoneMetaData(factoryId));
        
        return phone;
    }
    
    private Phone loadPhoneFromFactory(PhoneMetaData meta) {
        Phone phone = meta.getPhone();
        if (phone == null) {
            phone = (Phone) m_beanFactory.getBean(meta.getFactoryId());
            phone.setPhoneMetaData(meta);
            meta.setPhone(phone);
        }
        
        return phone;        
    }

    private Line loadLineFromFactory(LineMetaData meta) {
        Line line = meta.getLine();
        if (line == null) {
            line = loadPhoneFromFactory(meta.getPhoneMetaData()).createLine();
            line.setLineMetaData(meta);
            meta.setLine(line);
        }
        
        return line;        
    }

    public Object load(Class c, Integer id) {
        return getHibernateTemplate().load(c, id);
    }
    
    public Folder loadRootPhoneFolder() {
       return m_settingDao.loadRootFolder(PhoneMetaData.FOLDER_RESOURCE_NAME);
    }
    
    public Folder loadRootLineFolder() {
        return m_settingDao.loadRootFolder(LineMetaData.FOLDER_RESOURCE_NAME);
    }    

    /** unittesting only */
    public void clear() {
        getHibernateTemplate().delete("from PhoneMetaData");
        getHibernateTemplate().delete("from Folder");
        getHibernateTemplate().delete("from ValueStorage");
    }
}
