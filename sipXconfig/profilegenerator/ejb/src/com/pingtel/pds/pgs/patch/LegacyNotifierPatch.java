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
package com.pingtel.pds.pgs.patch;

import java.rmi.RemoteException;
import java.util.Map;

import org.sipfoundry.sipxconfig.legacy.LegacyNotifyService;

import com.pingtel.pds.pgs.common.RMIConnectionManager;
import com.pingtel.pds.pgs.common.ejb.BaseEJB;

public class LegacyNotifierPatch implements Patch {
    
    private Map m_props;
    
    public String update(Integer patchNumber) {        
        String serviceUrl = BaseEJB.getPGSProperty("legacynotifyservice.rmi.url");
        LegacyNotifyService notifyService;
        try {
            notifyService = (LegacyNotifyService) RMIConnectionManager
                    .getInstance().getConnection(serviceUrl);
            notifyService.onApplyPatch(patchNumber, m_props);
        } catch (RemoteException e) {
            throw new RuntimeException(e);
        }

        return Patch.SUCCESS;
    }

    public void initialize(Map props) {
        m_props = props;
    }
}
