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

import java.util.Set;

public interface ModelFilesContext {
    public Setting loadModelFile(String basename);

    public Setting loadModelFile(String basename, String manufacturer);
  
    public Setting loadDynamicModelFile(String basename, String manufacturer, Set defines);
    
    public Setting loadDynamicModelFile(String basename, String manufacturer, SettingExpressionEvaluator evalutor);
}
