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
package org.sipfoundry.sipxconfig.site.admin;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.commons.logging.LogFactory;
import org.apache.hivemind.Messages;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageBeginRenderListener;
import org.apache.tapestry.html.BasePage;
import org.apache.tapestry.request.IUploadFile;
import org.apache.tapestry.valid.ValidationConstraint;
import org.sipfoundry.sipxconfig.bulk.BulkManager;
import org.sipfoundry.sipxconfig.components.SipxValidationDelegate;
import org.sipfoundry.sipxconfig.components.TapestryUtils;

public abstract class BulkImport extends BasePage implements PageBeginRenderListener {

    public abstract IUploadFile getUploadFile();

    public abstract BulkManager getBulkManager();

    public void pageBeginRender(PageEvent event_) {
    }

    public void submit(IRequestCycle cycle_) {
        if (!TapestryUtils.isValid(this)) {
            return;
        }
        SipxValidationDelegate validator = (SipxValidationDelegate) TapestryUtils
                .getValidator(this);
        IUploadFile uploadFile = getUploadFile();
        String filePath = uploadFile.getFilePath(); 
        if (StringUtils.isBlank(filePath)) {
            validator.record(getMessages().getMessage("msg.errorNoFile"), ValidationConstraint.REQUIRED);
            return;
        }
        OutputStream os = null;
        try {
            File tmpFile = File.createTempFile("csv_import", null);
            os = new FileOutputStream(tmpFile);
            IOUtils.copy(uploadFile.getStream(), os);
            os.close();
            getBulkManager().insertFromCsv(tmpFile, true);
            validator.recordSuccess(getMessages().getMessage("msg.success"));
        } catch (IOException e) {
            LogFactory.getLog(getClass()).error("Cannot import file", e);
            Messages messages = getMessages();
            validator.record(messages.format("msg.error", e.getLocalizedMessage()),
                    ValidationConstraint.CONSISTENCY);
        } finally {
            IOUtils.closeQuietly(os);
        }
    }
}
