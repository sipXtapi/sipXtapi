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
package org.sipfoundry.sipxconfig.core;

/**
 * System-wide access to plugins
 */
public interface DevicePlugin {

    public DeviceModel[] getModels();

    public DeviceGenerator getGenerator(DeviceModel model);

    public DevicePublisher getPublisher(DeviceModel model);

    public DeviceWriter getWriter(DeviceModel model);
}
