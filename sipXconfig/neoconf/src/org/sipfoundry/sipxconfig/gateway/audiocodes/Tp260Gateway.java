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
package org.sipfoundry.sipxconfig.gateway.audiocodes;

import org.sipfoundry.sipxconfig.setting.Setting;

public class Tp260Gateway extends AudioCodesGateway {
    
    @Override
    protected Setting loadSettings() {
        return getModelFilesContext().loadModelFile("tp260-gateway.xml", "audiocodes");
    }
}
