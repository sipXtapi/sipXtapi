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
package org.sipfoundry.sipxconfig.components;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;

import org.apache.commons.io.CopyUtils;
import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.event.PageEvent;
import org.apache.tapestry.event.PageRenderListener;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.apache.tapestry.request.IUploadFile;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;

public abstract class AssetSelector extends BaseComponent implements PageRenderListener {
    public abstract String getAssetDir();

    public abstract void setAssetSelectionModel(IPropertySelectionModel model);

    public abstract IUploadFile getUploadAsset();

    public abstract void setUploadAsset(IUploadFile uploadFile);

    public abstract void setAsset(String asset);

    public abstract String getAsset();

    public void pageBeginRender(PageEvent event_) {
        File assetDir = new File(getAssetDir());
        String[] assets = assetDir.list();
        if (assets == null) {
            assets = new String[0];
        }

        IPropertySelectionModel model = new StringPropertySelectionModel(assets);
        setAssetSelectionModel(model);
    }

    private static boolean isUploadFileSpecified(IUploadFile file) {
        boolean isSpecified = file != null && !StringUtils.isBlank(file.getFileName());
        return isSpecified;
    }

    public void checkFileUpload() {
        IUploadFile upload = getUploadAsset();
        if (!isUploadFileSpecified(upload)) {
            return;
        }

        FileOutputStream promptWtr = null;
        try {
            File promptsDir = new File(getAssetDir());
            promptsDir.mkdirs();
            File promptFile = new File(promptsDir, upload.getFileName());
            promptWtr = new FileOutputStream(promptFile);
            CopyUtils.copy(upload.getStream(), promptWtr);
            setAsset(promptFile.getName());
            setUploadAsset(null);
        } catch (IOException ioe) {
            throw new RuntimeException("Could not upload file " + upload.getFileName(), ioe);
        } finally {
            IOUtils.closeQuietly(promptWtr);
        }
    }

    /**
     * Only call during validation phase
     * 
     * @param validator
     * @param errorMsg
     */
    public void validateNotEmpty(IValidationDelegate validator, String errorMsg) {
        if (StringUtils.isBlank(getAsset()) && !isUploadFileSpecified(getUploadAsset())) {
            validator.record(errorMsg, ValidationConstraint.REQUIRED);
        }
    }
}
