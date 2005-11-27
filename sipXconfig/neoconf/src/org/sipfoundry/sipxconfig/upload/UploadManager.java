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

import org.sipfoundry.sipxconfig.common.DataObjectSource;

public interface UploadManager extends DataObjectSource {
    
    public static final String CONTEXT_BEAN_NAME = "uploadManager";
    
    public Upload loadUpload(Integer uploadId);
    
    public void saveUpload(Upload upload);
    
    public void deleteUpload(Upload upload);
    
    public List getUploadSpecifications();

    public List getUpload();
    
    public Upload newUpload(UploadSpecification manufacturer);

}
