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
package org.sipfoundry.sipxconfig.vendor.cisco;

import java.io.File;

import org.sipfoundry.sipxconfig.core.DeviceGenerator;
import org.sipfoundry.sipxconfig.core.DeviceModel;
import org.sipfoundry.sipxconfig.core.DevicePlugin;
import org.sipfoundry.sipxconfig.core.DevicePublisher;
import org.sipfoundry.sipxconfig.core.DeviceWriter;

/**
 * Comments
 */
public class CiscoDevicePlugin implements DevicePlugin {

    static final DeviceModel[] MODELS = {
        new DeviceModel("7905"), 
        new DeviceModel("7912"),
        new DeviceModel("7940"),
        new DeviceModel("7960") 
    };

    private DeviceGenerator m_generatorTemp;

    private DevicePublisher m_publisher;

    private DeviceWriter m_writer;

    public DeviceModel[] getModels() {
        return MODELS;
    }

    public DeviceGenerator getGenerator(DeviceModel model) {
        // TODO: cache by model
        return new CiscoDeviceGenerator(model);
    }

    public DevicePublisher getPublisher(DeviceModel modelTemp) {
        return m_publisher;
    }

    public DeviceWriter getWriter(DeviceModel modelTemp) {
        return m_writer;
    }

}

class CiscoDeviceGenerator implements DeviceGenerator {

    private DeviceModel m_model;

    CiscoDeviceGenerator(DeviceModel model) {
        m_model = model;
    }

    /**
     * XML filename that describes a particular model's definitions
     * 
     * @param model
     * @return filepath to xml file
     */
    public String getDefinitions() {
        return new StringBuffer().append(File.separatorChar).append(m_model.getModel()).append(
                "definitions.xml").toString();
    }

}
