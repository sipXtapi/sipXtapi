/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import java.io.File;
import java.io.IOException;

import org.sipfoundry.sipxconfig.setting.SettingGroup;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

/**
 * Generic phone does not correlate to any particular phone. It represents any unsupported phones
 * that simply needs to be configured with basic network settings to the commservers so it can
 * make a calls. Phone is responsible configuring itself matching the configuration on the
 * commservers
 */
public class GenericPhone implements Phone {

    public static final String GENERIC_PHONE_ID = "generic";

    private String m_id = GENERIC_PHONE_ID;

    private Endpoint m_endpoint;
    
    private String m_systemDirectory;
    
    private String m_lineModelFilename;
    
    private String m_endpointModelFilename;
    
    private SettingGroup m_endpointModel;

    private SettingGroup m_lineModel;

    public String getModelId() {
        return m_id;
    }

    public String getDisplayLabel() {
        return "Generic SIP Device";
    }

    public void setModelId(String id) {
        m_id = id;
    }

    public void setEndpoint(Endpoint endpoint) {
        m_endpoint = endpoint;
    }
    
    public Endpoint getEndpoint() {
        return m_endpoint;
    }

    public SettingGroup getSettingModel(Endpoint endpoint_) {
        if (m_endpointModel == null) {
            File modelDefsFile = getFile(getSystemDirectory(), getEndpointModelFilename());
            m_endpointModel = new XmlModelBuilder().buildModel(modelDefsFile);
        }

        return m_endpointModel;
    }

    public SettingGroup getSettingModel(Line line_) {
        if (m_lineModel == null) {
            File lineDefsFile = getFile(getSystemDirectory(), getLineModelFilename());
            m_lineModel = new XmlModelBuilder().buildModel(lineDefsFile);
        }

        return m_lineModel;
    }

    /**
     * Not applicable
     */
    public void generateProfiles(PhoneContext phoneContext_, Endpoint endpoint_) throws IOException {        
    }
    
    /**
     * Not applicable
     */
    public void restart(PhoneContext phoneContext_, Endpoint endpoint_) throws IOException {
    }
    
    public String getEndpointModelFilename() {
        return m_endpointModelFilename;
    }

    public void setEndpointModelFilename(String endpointModelFilename) {
        m_endpointModelFilename = endpointModelFilename;
    }

    public String getSystemDirectory() {
        return m_systemDirectory;
    }

    public void setSystemDirectory(String systemDirectory) {
        m_systemDirectory = systemDirectory;
    }

    protected File getFile(String root, String filename) {
        return new File(root + '/' + filename);
    }

    public String getLineModelFilename() {
        return m_lineModelFilename;
    }

    public void setLineModelFilename(String lineModelFilename) {
        m_lineModelFilename = lineModelFilename;
    }

    
}