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

import org.sipfoundry.sipxconfig.common.DataObjectSource;

public interface UploadManager extends DataObjectSource {
    
    public static final String CONTEXT_BEAN_NAME = "uploadManager";
    
    public Upload loadUpload(Integer uploadId);
    
    public void saveUpload(Upload upload);
    
    public void deleteUpload(Upload upload);
    
    public Collection<Upload> getUpload();
    
    public Upload newUpload(UploadSpecification manufacturer);
    
    public boolean isActiveUploadById(UploadSpecification spec);
    
    public UploadSpecification getSpecification(String specId);
    
    /**
     * Checks to ensure you're not deploying more than one type of upload
     * then delegates deployment to upload object
     * then saves the upload state is deployment was successful
     */
    public void deploy(Upload upload);
    
    /**
     * Delegates undeployment to upload object
     * then saves the upload state is undeployment was successful
     */
    public void undeploy(Upload upload);
    
    /**
     * testing only
     */
    public void clear();
}
