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
import java.util.Set;

public class ModelFilesContextImpl implements ModelFilesContext {
    private String m_configDirectory;

    private ModelBuilder m_modelBuilder;

    // AOP NOTE: methods matching loadModelFile are cacheable    
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
    // AOP NOTE: methods matching loadModelFile are cacheable    
    public Setting loadModelFile(String basename, String manufacturer) {
        File modelFile = getModelFile(basename, manufacturer);
        Setting model = m_modelBuilder.buildModel(modelFile, null);
        return model;
    }

    /**
     * return a deep copy of this setting set that evaluate if/unless expressions that are
     * contained in this set. Example: if Set = { "Hi" } then settings/groups with if="Hi" will be
     * included settings/groups with unless="Hi" will not be included
     */
    // AOP NOTE: methods matching loadModelFile are cacheable, this intentionally is not    
    public Setting loadDynamicModelFile(String basename, String manufacturer, Set defines) {
        return loadDynamicModelFile(basename, manufacturer, new SimpleDefinitionsEvaluator(defines));
    }

    // AOP NOTE: methods matching loadModelFile are cacheable, this intentionally is not    
    public Setting loadDynamicModelFile(String basename, String manufacturer, SettingExpressionEvaluator evalutor) {
        Setting model = null;
        Setting master = loadModelFile(basename, manufacturer);
        if (master != null) {
            ConditionalSet conditional = (ConditionalSet) master;
            model = conditional.evaluate(evalutor);
        }
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

    /**
     * look for expression in the set
     */
    static class SimpleDefinitionsEvaluator implements SettingExpressionEvaluator {
        private Set m_defines;

        public SimpleDefinitionsEvaluator(Set defines) {
            m_defines = defines;
        }

        public boolean isExpressionTrue(String expression, Setting setting_) {
            return m_defines.contains(expression);
        }
    }
}
