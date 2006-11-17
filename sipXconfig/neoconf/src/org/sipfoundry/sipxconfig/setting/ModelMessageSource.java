/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.setting;

import java.io.File;

import org.apache.commons.io.FilenameUtils;
import org.springframework.context.support.ResourceBundleMessageSource;

/**
 * Special type of message source that finds it's resource bundle in the same plae where model
 * files are stored.
 */
public class ModelMessageSource extends ResourceBundleMessageSource {
    public ModelMessageSource(File modelFile) {
        setBasename(getBundleBasename(modelFile));
        setBundleClassLoader(getClass().getClassLoader());
    }

    /**
     * Find name for message bundle based on the name of the model file.
     * 
     * The assumption here is that the parent directory is in the classpath. If the name of the
     * file is '/etc/sipxpbx/polycom/phone.xml' we conver it to polycom.phone because we assume
     * that /etc/sipxpbx is in the classpath.
     * 
     * This will only work of course if model files are kept in subdirectories. It would be better
     * if both model class and resource bundles were loaded as resources using the same mechanism.
     * 
     * @param modelFile name of the .xml file containing the phone model, this is usually a name
     *        somewhere in /etc/sipxpbx directory
     * 
     * @return name of the bundle which has to look like a class name hence '.' as an separator
     */
    private String getBundleBasename(File modelFile) {
        String parentName = modelFile.getParentFile().getName();
        String baseName = FilenameUtils.getBaseName(modelFile.getName());
        return parentName + "." + baseName;
    }

}
