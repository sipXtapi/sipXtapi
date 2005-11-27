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
    
    private List m_uploadSpecifications;

    /**
     * Callback that supplies the owning factory to a bean instance.
     */
    public void setBeanFactory(BeanFactory beanFactory) {
        m_beanFactory = beanFactory;
    }

    public Upload loadUpload(Integer uploadId) {
        return (Upload) getHibernateTemplate().load(Upload.class, uploadId);
    }

    public void saveUpload(Upload upload) {
        getHibernateTemplate().saveOrUpdate(upload);            
    }

    public void deleteUpload(Upload upload) {
        upload.remove();
        getHibernateTemplate().delete(upload);
    }

    public List getUploadSpecifications() {
        return m_uploadSpecifications;
    }
    
    public void setUploadSpecifications(List uploadSpecifications) {
        m_uploadSpecifications = uploadSpecifications;
    }
    
    public List getUpload() {
        return getHibernateTemplate().findByNamedQuery("upload");
    }
        
    public Upload newUpload(UploadSpecification specification) {       
        Upload f = (Upload) m_beanFactory.getBean("uploader");
        f.setSpecification(specification);
        return f;
    }
}
