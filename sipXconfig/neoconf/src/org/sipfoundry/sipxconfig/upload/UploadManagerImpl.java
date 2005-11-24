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
package org.sipfoundry.sipxconfig.upload;

import java.util.List;

import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.springframework.beans.factory.BeanFactory;
import org.springframework.beans.factory.BeanFactoryAware;

public class UploadManagerImpl extends SipxHibernateDaoSupport implements BeanFactoryAware, UploadManager {
    private BeanFactory m_beanFactory;
    
    private List m_firmwareManufacturers;

    /**
     * Callback that supplies the owning factory to a bean instance.
     */
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public Firmware loadFirmware(Integer firmwareId) {
        return (Firmware) getHibernateTemplate().load(Firmware.class, firmwareId);
    }

    public void saveFirmware(Firmware firmware) {
        getHibernateTemplate().saveOrUpdate(firmware);            
    }

    public void deleteFirmware(Firmware firmware) {
        firmware.remove();
        getHibernateTemplate().delete(firmware);
    }

    public List getFirmwareManufacturers() {
        return m_firmwareManufacturers;
    }
    
    public void setFirmwareManufacturers(List firmwareManufacturers) {
        m_firmwareManufacturers = firmwareManufacturers;
    }
    
    public List getFirmware() {
        return getHibernateTemplate().findByNamedQuery("firmware");
    }
        
    public Firmware newFirmware(FirmwareManufacturer manufacturer) {       
        Firmware f = (Firmware) m_beanFactory.getBean("uploader");
        f.setManufacturer(manufacturer);
        return f;
    }
}
