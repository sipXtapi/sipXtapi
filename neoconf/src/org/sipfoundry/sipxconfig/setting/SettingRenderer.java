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
package org.sipfoundry.sipxconfig.setting;

public interface SettingRenderer {
    
    /**
     * Properties of a setting, 
     * @return null for no special processing  
     */
    public RenderProperties getRenderProperties(Setting setting);

}

