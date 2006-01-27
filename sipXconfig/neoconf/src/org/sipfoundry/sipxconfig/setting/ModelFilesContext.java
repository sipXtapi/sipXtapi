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

public interface ModelFilesContext {
    public Setting loadModelFile(String basename);

    public Setting loadModelFile(String basename, String manufacturer);

    public File getModelFile(String basename, String manufacturer);
}
