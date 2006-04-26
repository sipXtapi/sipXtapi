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
package org.sipfoundry.sipxconfig.setting;

import java.io.File;

public class ModelFilesContextImpl implements ModelFilesContext {
    private String m_configDirectory;

    private ModelBuilder m_modelBuilder;

    public Setting loadModelFile(String basename) {
        File modelFile = getModelFile(basename, null);
        SettingSet model = m_modelBuilder.buildModel(modelFile, null);
        return model.copy();
    }

    /**
     * loadModelFile("phone.xml", "cisco", new String[] { "7912" }) Loads settings model from XML
     * file
     * 
     * The full path of the model file is: systemEtcDirectory/manufacturer/basename
     * 
     * @return new copy of the settings model
     */
    public Setting loadModelFile(String basename, String manufacturer) {
        File modelFile = getModelFile(basename, manufacturer);
        Setting model = m_modelBuilder.buildModel(modelFile, null);
        return model;
    }

    File getModelDirectory(String manufacturer) {
        File modelDir;
        if (manufacturer == null) {
            modelDir = new File(m_configDirectory);
        } else {
            modelDir = new File(m_configDirectory, manufacturer);
        }
        return modelDir;
    }

    /**
     * @param manufacturer null if model file is in root directory
     */
    public File getModelFile(String basename, String manufacturer) {
        File modelDir = getModelDirectory(manufacturer);
        return new File(modelDir, basename);
    }

    public void setConfigDirectory(String configDirectory) {
        m_configDirectory = configDirectory;
    }

    public void setModelBuilder(ModelBuilder modelBuilder) {
        m_modelBuilder = modelBuilder;
    }
}
