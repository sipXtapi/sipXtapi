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

import java.io.File;
import java.util.Collection;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;

import net.sf.hibernate.Hibernate;

import org.apache.commons.collections.map.LinkedMap;
import org.sipfoundry.sipxconfig.common.BeanWithId;
import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.setting.Folder;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingDao;
import org.sipfoundry.sipxconfig.setting.ValueStorage;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;
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

    private Map m_phoneIds;

    private JobQueue m_jobQueue;

    private String m_systemDirectory;

    private CoreContext m_coreContext;

    private Map m_modelCache = new HashMap();

    /**
     * Generate profile on phones in background
     */
    public void generateProfilesAndRestart(Collection phones) {
        JobRecord job = createJobRecord(phones, JobRecord.TYPE_PROJECTION);
        m_jobQueue.addJob(job);
    }

    /**
     * Restart phones in background
     */
    public void restart(Collection phones) {
        JobRecord job = createJobRecord(phones, JobRecord.TYPE_DEVICE_RESTART);
        m_jobQueue.addJob(job);
    }

    JobRecord createJobRecord(Collection phones, int type) {
        JobRecord job = new JobRecord();
        job.setType(type);
        Phone[] phonesArray = (Phone[]) phones.toArray(new Phone[0]);
        job.setPhones(phonesArray);

        return job;
    }

    public void setSettingDao(SettingDao settingDao) {
        m_settingDao = settingDao;
    }

    public void setJobQueue(JobQueue jobQueue) {
        m_jobQueue = jobQueue;
    }

    /**
     * Callback that supplies the owning factory to a bean instance.
     */
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public Map getPhoneFactoryIds() {
        return m_phoneIds;
    }

    public void setPhoneFactoryIds(Map phoneIds) {
        m_phoneIds = phoneIds;
    }

    public void flush() {
        getHibernateTemplate().flush();
    }

    public void storePhone(Phone phone) {        
        PhoneData meta = phone.getPhoneData();
        checkForDuplicateFields("A phone with this serial number already exists", meta, 
            "serialNumber", meta.getSerialNumber());
        checkForDuplicateFields("A phone with this name already exists", meta, 
                "name", meta.getName());
        meta.setValueStorage(clearUnsavedValueStorage(meta.getValueStorage()));
        getHibernateTemplate().saveOrUpdate(meta);
        Iterator lines = phone.getLines().iterator();
        while (lines.hasNext()) {
            Line line = (Line) lines.next();
            storeLine(line);
        }
        Iterator deleted = phone.getDeletedLines().iterator();
        while (deleted.hasNext()) {
            Line line = (Line) deleted.next();
            deleteLine(line);
        }
    }
    
    void  checkForDuplicateFields(String message, BeanWithId obj, String field, Object value) {
        if (value == null) {
            return;
        }
        
        String name = obj.getClass().getName();
        String uniqueCheck = "select o.id from " + name + " o where o." + field + " = :value";
        List ids = getHibernateTemplate().findByNamedParam(uniqueCheck, "value", value);
        
        // no match 
        if (ids.size() == 0) {
            return;            
        }

        // detect 1 match, itself
        if (!obj.isNew() && ids.size() == 1) {
            Integer id = (Integer) ids.get(0);
            if (id.equals(obj.getId())) {
                return;            
            }
        }
        
        throw new DuplicateFieldException(message);
    }

    public void deletePhone(Phone phone) {
        Iterator lines = phone.getLines().iterator();
        while (lines.hasNext()) {
            deleteLine((Line) lines.next());
        }
        Iterator deleted = phone.getDeletedLines().iterator();
        while (deleted.hasNext()) {
            deleteLine((Line) deleted.next());
        }
        PhoneData meta = phone.getPhoneData();
        meta.setValueStorage(clearUnsavedValueStorage(meta.getValueStorage()));
        getHibernateTemplate().delete(meta);
    }

    public void storeLine(Line line) {
        LineData meta = line.getLineData();
        meta.setValueStorage(clearUnsavedValueStorage(meta.getValueStorage()));
        getHibernateTemplate().saveOrUpdate(meta);
    }

    public void deleteLine(Line line) {
        LineData meta = line.getLineData();
        meta.setValueStorage(clearUnsavedValueStorage(meta.getValueStorage()));
        getHibernateTemplate().delete(meta);
    }

    ValueStorage clearUnsavedValueStorage(ValueStorage vs) {
        // HACK: Load incase it needs to be deleted
        return vs != null && vs.getId() == UNSAVED_ID && vs.size() == 0 ? null : vs;
    }

    public Line loadLine(Integer id) {
        return loadLine((LineData) getHibernateTemplate().load(LineData.class, id));
    }

    public Collection loadPhones() {
        // Inventing a hibernate, transient object here to associate phone and line metadata
        // might be helpful.
        String phoneQuery = "from PhoneData p";
        List phoneMetas = getHibernateTemplate().find(phoneQuery);
        Map phones = new LinkedMap();
        for (int i = 0; i < phoneMetas.size(); i++) {
            PhoneData meta = (PhoneData) phoneMetas.get(i);
            Phone phone = loadPhoneFromFactory(meta);
            phones.put(meta.getPrimaryKey(), phone);
        }

        String lineQuery = "from LineData l order by l.phoneData, l.position asc";
        List lineMetas = getHibernateTemplate().find(lineQuery);
        for (int i = 0; i < lineMetas.size(); i++) {
            LineData lineMeta = (LineData) lineMetas.get(i);
            // collate by parent object: phoneMetaData
            Phone phone = (Phone) phones.get(lineMeta.getPhoneData().getPrimaryKey());
            if (phone == null) {
                phone = loadPhoneFromFactory(lineMeta.getPhoneData());
                phones.put(lineMeta.getPhoneData().getPrimaryKey(), phone);
            }
            phone.addLine(phone.createLine(lineMeta));
        }

        return phones.values();
    }

    public Phone loadPhone(Integer id) {
        Phone phone = loadPhoneFromFactory((PhoneData) getHibernateTemplate().load(
                PhoneData.class, id));
        String lineQuery = "from LineData l where l.phoneData = :phone order by l.position asc";
        List lineMetas = getHibernateTemplate().findByNamedParam(lineQuery, "phone",
                phone.getPhoneData());
        for (int i = 0; i < lineMetas.size(); i++) {
            LineData meta = (LineData) lineMetas.get(i);
            phone.addLine(phone.createLine(meta));
        }

        return phone;
    }

    public Phone newPhone(String factoryId) {
        Phone phone = (Phone) m_beanFactory.getBean(factoryId);
        phone.setPhoneData(new PhoneData(factoryId));
        phone.setPhoneContext(this);

        return phone;
    }

    public Line newLine(String factoryId) {
        return (Line) m_beanFactory.getBean(factoryId);
    }

    private Phone loadPhoneFromFactory(PhoneData meta) {
        Phone phone = (Phone) m_beanFactory.getBean(meta.getFactoryId());
        phone.setPhoneData(meta);
        phone.setPhoneContext(this);
        meta.setModelLabel((String) m_phoneIds.get(meta.getFactoryId()));

        return phone;
    }

    Line loadLine(LineData meta) {
        m_coreContext.loadUserPassword(meta.getUser());
        return loadPhoneFromFactory(meta.getPhoneData()).createLine(meta);
    }

    public Object load(Class c, Integer id) {
        return getHibernateTemplate().load(c, id);
    }

    public Folder loadRootPhoneFolder() {
        return m_settingDao.loadRootFolder(PhoneData.FOLDER_RESOURCE_NAME);
    }

    public Folder loadRootLineFolder() {
        return m_settingDao.loadRootFolder(LineData.FOLDER_RESOURCE_NAME);
    }

    public JobRecord loadJob(Integer id) {
        return (JobRecord) getHibernateTemplate().load(JobRecord.class, id);
    }

    public void storeJob(JobRecord job) {
        getHibernateTemplate().saveOrUpdate(job);
    }

    /** unittesting only */
    public void clear() {
        // ordered bottom-up, e.g. traverse foreign keys so as to
        // not leave hanging references. DB will reject otherwise
        getHibernateTemplate().delete("from LineData");
        getHibernateTemplate().delete("from PhoneData");
        getHibernateTemplate().delete("from Folder");
        getHibernateTemplate().delete("from ValueStorage");
    }

    public String getSystemDirectory() {
        return m_systemDirectory;
    }

    public void setSystemDirectory(String systemDirectory) {
        m_systemDirectory = systemDirectory;
    }

    public String getDnsDomain() {
        return m_coreContext.loadRootOrganization().getDnsDomain();
    }

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }

    public Setting getSettingModel(String filename) {
        // cache it, but may be helpful to reload model on fly in future
        Setting model = (Setting) m_modelCache.get(filename);
        if (model == null) {
            File modelDefsFile = new File(getSystemDirectory() + '/' + filename);
            model = new XmlModelBuilder(getSystemDirectory()).buildModel(modelDefsFile);
            m_modelCache.put(filename, model);
        }
        return model;
    }

    public void deleteLinesForUser(Integer userId) {
        // TODO: move query to .hbm.xml file
        getHibernateTemplate().delete("from LineData line where line.user.id=?", userId,
                Hibernate.INTEGER);
    }
}
