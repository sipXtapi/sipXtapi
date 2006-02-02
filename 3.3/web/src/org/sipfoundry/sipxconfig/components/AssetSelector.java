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

import org.apache.commons.io.IOUtils;
import org.apache.commons.lang.StringUtils;
import org.apache.tapestry.AbstractPage;
import org.apache.tapestry.BaseComponent;
import org.apache.tapestry.IForm;
import org.apache.tapestry.IMarkupWriter;
import org.apache.tapestry.IRequestCycle;
import org.apache.tapestry.form.IFormComponent;
import org.apache.tapestry.form.IPropertySelectionModel;
import org.apache.tapestry.form.StringPropertySelectionModel;
import org.apache.tapestry.request.IUploadFile;
import org.apache.tapestry.valid.IValidationDelegate;
import org.apache.tapestry.valid.ValidationConstraint;

public abstract class AssetSelector extends BaseComponent implements IFormComponent {
    public abstract String getAssetDir();

    public abstract void setAssetSelectionModel(IPropertySelectionModel model);

    public abstract IUploadFile getUploadAsset();

    public abstract void setUploadAsset(IUploadFile uploadFile);

    public abstract void setAsset(String asset);

    public abstract String getAsset();

    public abstract String getErrorMsg();
    
    public abstract String getContentType();
    
    public abstract boolean getSelectable();
    
    public abstract String getDeleteAsset();

    public abstract void setDeleteAsset(String asset);

    private static boolean isUploadFileSpecified(IUploadFile file) {
        boolean isSpecified = file != null && !StringUtils.isBlank(file.getFilePath());
        return isSpecified;
    }
    
    public boolean getAssetExists() {
        return StringUtils.isNotBlank(getAsset());
    }

    public String getDownloadLabel() {
        String key = "download.general";
        if (getContentType().startsWith("audio/")) {
            key = "download.audio";
        }
        String localized = getMessages().getMessage(key);
        return localized;
    }
    
    private IPropertySelectionModel createSelectableAssetModel() {
        File assetDir = new File(getAssetDir());
        // make sure it exists
        assetDir.mkdirs();
        String[] assets = assetDir.list();
        if (assets == null) {
            assets = new String[0];
        }
        return new StringPropertySelectionModel(assets);
    }
    
    protected void renderComponent(IMarkupWriter writer, IRequestCycle cycle) {
        IPropertySelectionModel model = createSelectableAssetModel();
        setAssetSelectionModel(model);
        super.renderComponent(writer, cycle);
        if (!cycle.isRewinding()) {
            return;
        }
        AbstractPage page = (AbstractPage) getPage();
        IValidationDelegate validator = TapestryUtils.getValidator(page);
        validateNotEmpty(validator, getErrorMsg());
        TapestryUtils.isValid(page);
        checkFileUpload();
        checkDeleteAsset();        
    }
    
    private void checkDeleteAsset() {
        if (getDeleteAsset() != null) {
            File assetFile = new File(getAssetDir(), getDeleteAsset());
            assetFile.delete();
            setAsset(null);
            setDeleteAsset(null);
        }
    }
    
    private void checkFileUpload() {
        IUploadFile upload = getUploadAsset();
        if (!isUploadFileSpecified(upload)) {
            return;
        }

        FileOutputStream promptWriter = null;
        String fileName = getSystemIndependentFileName(upload.getFilePath());
        try {
            File promptsDir = new File(getAssetDir());
            promptsDir.mkdirs();
            File promptFile = new File(promptsDir, fileName);
            promptWriter = new FileOutputStream(promptFile);
            IOUtils.copy(upload.getStream(), promptWriter);
            setAsset(promptFile.getName());
            setUploadAsset(null);
        } catch (IOException ioe) {
            throw new RuntimeException("Could not upload file " + fileName, ioe);
        } finally {
            IOUtils.closeQuietly(promptWriter);
        }
    }

    /**
     * Extract file name from the path in a system independed way.
     * 
     * C:\a\b\c.txt -> c.txt a/b/c.txt => c.txt
     * 
     * We cannot use File.getName() here since it only works for filenames from the same operating
     * system. We have to handle the case when Windows file is downloaded on Linux server and vice
     * versa
     * 
     * @param filePath full name of the downloaded file in a client sytem format
     * @return base name and extension of the file
     */
    static String getSystemIndependentFileName(String filePath) {
        if (StringUtils.isEmpty(filePath)) {
            return StringUtils.EMPTY;
        }
        String[] parts = StringUtils.split(filePath, ":/\\");
        return parts[parts.length - 1];
    }

    /**
     * Only call during validation phase
     * 
     * @param validator
     * @param errorMsg - if empty we will not validate, if not empty we will record this message
     *        as an error in the validator
     */
    private void validateNotEmpty(IValidationDelegate validator, String errorMsg) {
        if (StringUtils.isEmpty(errorMsg)) {
            return;
        }
        if (StringUtils.isBlank(getAsset()) && !isUploadFileSpecified(getUploadAsset())) {
            validator.record(errorMsg, ValidationConstraint.REQUIRED);
        }
    }

    // what follows is empty IFormComponent implementation
    // we only need getDisplayName to return null - it's used by label component

    public IForm getForm() {
        return null;
    }

    public void setForm(IForm form_) {
        // empty
    }

    public String getName() {
        return StringUtils.EMPTY;
    }

    public void setName(String name_) {
    }

    /**
     * Implemented in some subclasses to provide a display name (suitable for presentation to the
     * user as a label or error message). This implementation return null.
     * 
     */

    public String getDisplayName() {
        return null;
    }
}
