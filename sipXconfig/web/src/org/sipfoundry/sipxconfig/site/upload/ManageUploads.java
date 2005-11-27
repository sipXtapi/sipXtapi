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

import java.util.Collection;

import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.html.BasePage;
import org.sipfoundry.sipxconfig.components.TapestryUtils;
import org.sipfoundry.sipxconfig.upload.UploadManager;
import org.sipfoundry.sipxconfig.upload.UploadSpecification;

public abstract class ManageUploads extends BasePage implements PageRenderListener {
    
    public static final String PAGE = "ManageUploads";

    public abstract void setUpload(Collection upload);

    public abstract Collection getUpload();
    
    public abstract UploadManager getUploadManager();
    
    public abstract UploadSpecification getSelectedSpecification();
    
    public void editUpload(IRequestCycle cycle) {
        Integer uploadId = (Integer) TapestryUtils.assertParameter(Integer.class, 
                cycle.getServiceParameters(), 0);
        EditUpload page = (EditUpload) cycle.getPage(EditUpload.PAGE);
        page.setUploadId(uploadId);
        cycle.activate(page);
    }
    
    public void addUpload(IRequestCycle cycle) {
        EditUpload page = (EditUpload) cycle.getPage(EditUpload.PAGE);
        page.setUploadId(null);
        page.setUploadSpecification(getSelectedSpecification());
        page.activatePageWithCallback(PAGE, cycle);
    }
    
    public void deleteUpload(IRequestCycle cycle_) {
    }

    /** stub: side-effect of PageRenderListener */
    public void pageBeginRender(PageEvent event_) {
        if (getUpload() == null) {
            setUpload(getUploadManager().getUpload());
        }
    }
    
    public void pageEndRender(PageEvent event) { 
        if (getSelectedSpecification() != null) {
            addUpload(event.getRequestCycle());
        }
    }
}
