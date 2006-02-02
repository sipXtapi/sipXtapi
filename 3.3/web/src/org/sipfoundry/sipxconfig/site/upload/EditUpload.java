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
package org.sipfoundry.sipxconfig.site.upload;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.sipfoundry.sipxconfig.components.PageWithCallback;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.upload.Upload;
import org.sipfoundry.sipxconfig.upload.UploadManager;
import org.sipfoundry.sipxconfig.upload.UploadSpecification;

public abstract class EditUpload extends PageWithCallback implements PageRenderListener {
    
    public static final String PAGE = "EditUpload";
    
    public abstract Upload getUpload();
    
    public abstract void setUpload(Upload upload);
    
    public abstract Integer getUploadId();
    
    public abstract void setUploadId(Integer id);
    
    public abstract UploadSpecification getUploadSpecification();
    
    public abstract void setUploadSpecification(UploadSpecification specification);
    
    public abstract UploadManager getUploadManager();
    
    public abstract boolean isActive();
    
    public abstract void setActive(boolean deployed);
    
    public void pageBeginRender(PageEvent event_) {
        Upload upload = getUpload();
        if (upload == null) {
            Integer id = getUploadId();
            if (id == null) {
                upload = getUploadManager().newUpload(getUploadSpecification());
            } else {
                upload = getUploadManager().loadUpload(id);
            }
            setUpload(upload);
            setActive(upload.isDeployed());
        }
    }
    
    private void checkDeploymentStatus() {
        Upload upload = getUpload();
        if (isActive() != upload.isDeployed()) {
            if (!isActive()) {
                getUploadManager().undeploy(upload);
            } else if (isActive()) {
                getUploadManager().deploy(upload);
            }
            setActive(upload.isDeployed());
        }        
    }
    
    public void onSave(IRequestCycle cycle_) {
        if (TapestryUtils.isValid(this)) {
            Upload upload = getUpload();
            getUploadManager().saveUpload(upload);
            setUploadId(upload.getId());           
            checkDeploymentStatus();            
        }
    }    
}
