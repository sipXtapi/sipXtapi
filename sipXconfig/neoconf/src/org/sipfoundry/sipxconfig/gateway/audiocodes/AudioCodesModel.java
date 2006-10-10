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

import java.io.File;

import org.sipfoundry.sipxconfig.gateway.GatewayModel;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;

public class AudioCodesModel extends GatewayModel {
    private String m_iniFileName;
    private String m_proxyNameSetting;
    private String m_proxyIpSetting;    
    private ModelFilesContext m_modelFilesContext;    
    
    public void setIniFileName(String iniFileName) {
        m_iniFileName = iniFileName;        
    }
    
    public File getIniFileTemplate() {
        return m_modelFilesContext.getModelFile(m_iniFileName, "audiocodes");        
    }
    
    public void setModelFilesContext(ModelFilesContext modelFilesContext) {
        m_modelFilesContext = modelFilesContext;
    }

    public String getProxyIpSetting() {
        return m_proxyIpSetting;
    }

    public void setProxyIpSetting(String proxyIpSetting) {
        m_proxyIpSetting = proxyIpSetting;
    }

    public String getProxyNameSetting() {
        return m_proxyNameSetting;
    }

    public void setProxyNameSetting(String proxyNameSetting) {
        m_proxyNameSetting = proxyNameSetting;
    }
}
