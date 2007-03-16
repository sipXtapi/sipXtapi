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

import java.util.Collection;
import java.util.List;

import org.sipfoundry.sipxconfig.common.SipxHibernateDaoSupport;
import org.sipfoundry.sipxconfig.common.UserException;
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
        saveBeanWithSettings(upload);
    }

    public void deleteUpload(Upload upload) {
        upload.remove();
        deleteBeanWithSettings(upload);
    }
    
    public Collection<Upload> getUpload() {
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

    public void deploy(Upload upload) {
        // ensure upload is saved first.  Allows it to create directory identifier 
        if (upload.isNew()) {
            saveUpload(upload);
        }
        
        UploadSpecification spec = upload.getSpecification();
        List existing = getHibernateTemplate().findByNamedQueryAndNamedParam(
                "deployedUploadBySpecification", "spec", spec.getSpecificationId());
        // should never happen
        if (existing.size() > 1) {
            throw new AlreadyDeployedException("There are already " + existing.size() 
                    + " files sets of type  \"" + spec.getLabel() + "\" deployed.  You can only have "
                    + " one set of files of this type deployed at a time");                
        }
        if (existing.size() == 1) {
            Upload existingUpload = (Upload) existing.get(0);
            if (!existingUpload.getId().equals(upload.getId())) {
                throw new AlreadyDeployedException("You must undeploy \"" +  existingUpload.getName()
                        + "\" before you can deploy these files.  You can only have one set of files of type \""
                        + spec.getLabel() + "\" deployed at a time.");
            }
        }
        
        upload.deploy();        
        saveUpload(upload);
    }

    public void undeploy(Upload upload) {
        upload.undeploy();
        saveUpload(upload);
    }

    static class AlreadyDeployedException extends UserException {
        AlreadyDeployedException(String msg) {
            super(msg);
        }
    }
    
}
