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
import java.io.IOException;
import java.io.InputStream;

public interface ModelBuilder {

    public abstract SettingSet buildModel(File modelFile, Setting parent);

    public abstract SettingSet buildModel(InputStream is, Setting parent) throws IOException;

}
