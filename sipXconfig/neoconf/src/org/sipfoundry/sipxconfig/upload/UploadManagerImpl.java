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
import org.springframework.beans.factory.ListableBeanFactory;

public class UploadManagerImpl extends SipxHibernateDaoSupport implements BeanFactoryAware, UploadManager {
    private ListableBeanFactory m_beanFactory;    

    /**
     * Callback that supplies the owning factory to a bean instance.
     */
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = (ListableBeanFactory) beanFactory;
    }

    public Upload loadUpload(Integer uploadId) {
        return (Upload) getHibernateTemplate().load(Upload.class, uploadId);
    }

    public void saveUpload(Upload upload) {
        upload.setValueStorage(clearUnsavedValueStorage(upload.getValueStorage()));
        getHibernateTemplate().saveOrUpdate(upload);            
    }

    public void deleteUpload(Upload upload) {
        upload.remove();
        
        // avoid hibernate errors about new object references when calling delete on parent object
        upload.setValueStorage(clearUnsavedValueStorage(upload.getValueStorage()));
        
        getHibernateTemplate().delete(upload);
    }

    public List getUploadSpecifications() {
        return UploadSpecification.getSpecifications();
    }
    
    public List getUpload() {
        return getHibernateTemplate().findByNamedQuery("upload");
    }
        
    public Upload newUpload(UploadSpecification specification) {       
        Upload upload = (Upload) m_beanFactory.getBean(specification.getBeanId());
        upload.setSpecificationId(specification.getSpecificationId());
        return upload;
    }

    public void clear() {
        getHibernateTemplate().deleteAll(getUpload());
    }
}
