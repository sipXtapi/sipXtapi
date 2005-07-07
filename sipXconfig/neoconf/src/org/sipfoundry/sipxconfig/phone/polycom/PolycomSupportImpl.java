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
package org.sipfoundry.sipxconfig.phone.polycom;

import java.io.File;

import org.sipfoundry.sipxconfig.common.CoreContext;
import org.sipfoundry.sipxconfig.common.User;
import org.sipfoundry.sipxconfig.legacy.LegacyContext;
import org.sipfoundry.sipxconfig.legacy.UserConfigSet;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;

/**
 * Support for Polycom phones
 */
public class PolycomSupportImpl implements PolycomSupport {

    private Setting m_lineModel;

    private Setting m_endpointModel;

    private String m_lineModelFilename = "polycom/line.xml";

    private String m_endpointModelFilename = "polycom/phone.xml";

    private String m_systemDirectory;
    
    private CoreContext m_coreContext; 
    
    private LegacyContext m_legacy;

    public Setting getLineSettingModel() {
        // cache it, but may be helpful to reload model on fly in future
        if (m_lineModel == null) {
            File modelDefsFile = getFile(getSystemDirectory(), getLineModelFilename());
            m_lineModel = new XmlModelBuilder().buildModel(modelDefsFile);
        }
        return m_lineModel;
    }

    public Setting getEndpointSettingModel() {
        // cache it, but may be helpful to reload model on fly in future
        if (m_endpointModel == null) {
            File modelDefsFile = getFile(getSystemDirectory(), getEndpointModelFilename());
            m_endpointModel = new XmlModelBuilder().buildModel(modelDefsFile);
        }
        return m_endpointModel;
    }

    public String getEndpointModelFilename() {
        return m_endpointModelFilename;
    }

    public void setEndpointModelFilename(String endpointModelFilename) {
        m_endpointModelFilename = endpointModelFilename;
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

    public String getSystemDirectory() {
        return m_systemDirectory;
    }

    public void setSystemDirectory(String systemDirectory) {
        m_systemDirectory = systemDirectory;
    }
    
    public String getDnsDomain() {
        return m_coreContext.loadRootOrganization().getDnsDomain();
    }

    public String getClearTextPassword(User user) {        
        UserConfigSet ucs = m_legacy.getConfigSetForUser(user);
        return ucs.getClearTextPassword();
    }

    public void setLegacyContext(LegacyContext legacy) {
        m_legacy = legacy;
    }    

    public void setCoreContext(CoreContext coreContext) {
        m_coreContext = coreContext;
    }
}
