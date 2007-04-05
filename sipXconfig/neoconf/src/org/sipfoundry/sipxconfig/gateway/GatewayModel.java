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
package org.sipfoundry.sipxconfig.gateway;

import org.sipfoundry.sipxconfig.device.DeviceDescriptor;

public class GatewayModel extends DeviceDescriptor {

    public GatewayModel() {
    }

    public GatewayModel(String beanId) {
        super(beanId);
    }

    public GatewayModel(String beanId, String modelId) {
        super(beanId, modelId);
    }
}
